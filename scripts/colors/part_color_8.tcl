clear
set datafile cad/179_synthetic_case.brep

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-brep $datadir/$datafile


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
set subDir "/part_color_8/"
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
# Note: Due to the fact that the face indices may have changed
#       when converting the model from BREP to STEP, the total
#       number of colored faces is checked.

set numColoredFaces 0
for {set i 1} {$i < 7} {incr i} {
  if { ![ catch { check-face-color -fid $i -color (255,0,0) } ] } {
   set numColoredFaces [expr $numColoredFaces + 1]
  }
}

if { $numColoredFaces != 1 } {
  return -code error "ERROR: The number of colored faces ($numColoredFaces) is not equal 1"
}

# Remove temporary files.
file delete -force $tmpDir