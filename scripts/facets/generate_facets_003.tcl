set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile

# Generate facets
generate-facets -lin 5.0 -ang 5.0

# Make temprary folder.
set subDir "/generate_facets_003/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
    file mkdir $tmpDir
}

# Save in gltf.
set resultName "result.glb"
save-gltf -filename $tmpDir$resultName

# Check size.
set expectedSize 126428
set precision 20
set size [file size $tmpDir$resultName]
if { [expr abs($expectedSize - $size) ] <= $precision } {
  file delete -force $tmpDir
} else {
  file delete -force $tmpDir
  return -code error "Error: actual size of $resultName is $size"
}