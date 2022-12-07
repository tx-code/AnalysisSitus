clear

set datafile points/PC2.xyz
set refPntsName reference/quickHull/ref_quick_hull_PC2.xyz

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-points pointsName $datadir/$datafile

# Build quick hull.
test-build-quick-hull pointsName -50.0 5.0 30.941698974854308 0.0 -1.0 0.0

# Make temprary folder.
set subDir "/quick_hull_01/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
  file mkdir $tmpDir
}

# Save point cloud.
set ptsName "hull-points"
set resultName "result.xyz"
save-xyz $ptsName $tmpDir$resultName

# Load auxiliary procedures.
set proceduresName auxiliary_procedures/points_procedures.tcl
if {[info procs ComparePointsCoordProc] eq ""} {
    puts "ComparePointsCoordProc does not exist.  sourcing $proceduresName"
	set scriptsdir $env(ASI_TEST_SCRIPTS)
    source $scriptsdir/$proceduresName
}

# Check pnts.
set tol 1.0e-5
ComparePointsCoordProc $tmpDir$resultName $datadir/$refPntsName $tol

# Remove temporary files.
file delete -force $tmpDir

set-as-part "hull"

test-check-number-shape-entities -vertex 184 -edge 184 -wire 1 -face 0 -shell 0 -solid 0 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 50.000000000000014 -yDim 0 -zDim 100.00000000000001 -tol 1.0e-4

clear