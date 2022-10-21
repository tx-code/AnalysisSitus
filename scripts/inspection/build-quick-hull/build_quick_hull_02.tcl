clear

set datafile points/PC3.xyz
set refPntsName reference/quickHull/ref_quick_hull_PC3.xyz

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-points pointsName $datadir/$datafile

# Build quick hull.
test-build-quick-hull pointsName -50.000000000000000 25.000000000000000 45.000000000000000 0.0000000000000000 0.0000000000000000 1.0000000000000000

# Make temprary folder.
set subDir "/quick_hull_02/"
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

test-check-number-shape-entities -vertex 11 -edge 11 -wire 1 -face 0 -shell 0 -solid 0 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 75.0 -yDim 25.0 -zDim 0.0 -tol 1.0e-4

clear