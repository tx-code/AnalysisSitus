set datafile cad/nist/nist_ctc_05.stp
set refPntsName reference/faceGrid/nist_ctc_05_117.xyz

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile

# Build face grid.
build-face-grid -num 100 -discr -fid 117

# Make temprary folder.
set subDir "/face_grid_08/"
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