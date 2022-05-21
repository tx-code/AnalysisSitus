set datafile cad/ANC101.stp

# Create box
make-box aaa

# Change color.
set-topo-item-color -name aaa -color (0,255,0)
check-topo-item-color -name aaa -color (0,255,0)

# Make temprary folder.
set subDir "/part_color_4/"
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
check-part-color -color (0,255,0)

# Remove temporary files.
file delete -force $tmpDir