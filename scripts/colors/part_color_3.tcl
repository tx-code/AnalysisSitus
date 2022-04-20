set datafile cad/ANC101_colored.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile

check-face-color -fid 82 -color (193,96,31)
check-face-color -fid 50 -color (40,94,131)

# Make temprary folder.
set subDir "/part_color_3/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
  file mkdir $tmpDir
}

# Save in new step.
set resultName "result.stp"
save-step $tmpDir$resultName

# Load saved step.
load-step $tmpDir$resultName

# Check color.
check-face-color -fid 82 -color (193,96,31)
check-face-color -fid 50 -color (40,94,131)

# Remove temporary files.
file delete -force $tmpDir