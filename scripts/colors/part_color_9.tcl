clear
set datafile cad/179_synthetic_case.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile


check-face-color -fid 1 -color (180,180,180)
check-face-color -fid 2 -color (180,180,180)
check-face-color -fid 3 -color (180,180,180)
check-face-color -fid 4 -color (180,180,180)
check-face-color -fid 5 -color (180,180,180)
check-face-color -fid 6 -color (180,180,180)

# Change colors and move part.
set-face-color -fid 2 -color (255,0,0)
set-face-color -fid 5 -color (255,0,0)
move-part 0 34 0 10 5 0

check-face-color -fid 1 -color (180,180,180)
check-face-color -fid 2 -color (255,0,0)
check-face-color -fid 3 -color (180,180,180)
check-face-color -fid 4 -color (180,180,180)
check-face-color -fid 5 -color (255,0,0)
check-face-color -fid 6 -color (180,180,180)

# Make temprary folder.
set subDir "/part_color_9/"
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

# Check colors.
check-face-color -fid 5 -color (255,0,0)
check-face-color -fid 2 -color (255,0,0)

check-face-color -fid 1 -color (180,180,180)
check-face-color -fid 3 -color (180,180,180)
check-face-color -fid 4 -color (180,180,180)
check-face-color -fid 6 -color (180,180,180)

# Remove temporary files.
file delete -force $tmpDir