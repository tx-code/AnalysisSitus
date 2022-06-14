clear
make-box a 0 0 0 1 1 1
set-as-part a

check-face-color -fid 1 -color (180,180,180)
check-face-color -fid 2 -color (180,180,180)
check-face-color -fid 3 -color (180,180,180)
check-face-color -fid 4 -color (180,180,180)
check-face-color -fid 5 -color (180,180,180)
check-face-color -fid 6 -color (180,180,180)

# Change color and move part.
set-face-color -fid 2 -color (255,0,0)
move-part 0 0 0 10 0 0

check-face-color -fid 1 -color (180,180,180)
check-face-color -fid 2 -color (255,0,0)
check-face-color -fid 3 -color (180,180,180)
check-face-color -fid 4 -color (180,180,180)
check-face-color -fid 5 -color (180,180,180)
check-face-color -fid 6 -color (180,180,180)

# Make temprary folder.
set subDir "/part_color_10/"
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
check-face-color -fid 2 -color (255,0,0)

check-face-color -fid 1 -color (180,180,180)
check-face-color -fid 3 -color (180,180,180)
check-face-color -fid 4 -color (180,180,180)
check-face-color -fid 5 -color (180,180,180)
check-face-color -fid 6 -color (180,180,180)

# Remove temporary files.
file delete -force $tmpDir