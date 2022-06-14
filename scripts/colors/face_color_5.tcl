clear
set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Recognize features and change color.
set holes [recognize-holes]
set nb [llength $holes]
set i 0
while { $i < $nb } {
  set-face-color -fid [lindex $holes $i] -color (255,0,0)
  incr i
}
check-face-color -fid 87 -color (255,0,0)

set cavities [recognize-cavities]
set nb [llength $cavities]
set i 0
while { $i < $nb } {
  set-face-color -fid [lindex $cavities $i] -color (0,255,0)
  incr i
}
check-face-color -fid 87 -color (0,255,0)

# Make temprary folder.
set subDir "/part_color_5/"
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
check-face-color -fid 87 -color (0,255,0)

# Remove temporary files.
file delete -force $tmpDir