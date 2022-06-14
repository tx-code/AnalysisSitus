clear
set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Change color.
set-face-color -fid 19 -color (255,0,0)
set-part-color -color (0,255,0)

check-face-color -fid 19 -color (255,0,0)
check-part-color -color (0,255,0)

# Make temprary folder.
set subDir "/part_color_2/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
  file mkdir $tmpDir
}

# Save in new step.
set resultName "result.stp"
save-step $tmpDir$resultName

# Load saved step.
clear
load-step $tmpDir$resultName

# Check color.
check-part-color -color (0,255,0)
check-face-color -fid 19 -color (255,0,0)

# Remove temporary files.
file delete -force $tmpDir