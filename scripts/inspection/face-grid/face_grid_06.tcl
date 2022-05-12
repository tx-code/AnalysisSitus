set datafile cad/anomalies/freecad/index_hmd_frunk.stp
set refPntsName reference/faceGrid/index_hmd_frunk_2934.xyz

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile

# Build face grid.
build-face-grid -num 100 -discr -fid 2934

# Make temprary folder.
set subDir "/face_grid_06/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
  file mkdir $tmpDir
}

# Save point cloud.
set ptsName "grid 3D"
set resultName "result.xyz"
save-xyz $ptsName $tmpDir$resultName

# Load auxiliary procedures.
set proceduresName inspection/face-grid/auxiliary_procedures.tcl
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