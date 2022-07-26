clear
set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile
generate-facets -lin 12.727 -ang 5

# Make temprary folder.
set subDir "/stl_write_1/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
    file mkdir $tmpDir
}

# Save in new STLs.
set resultName1 "result1.stl"
save-facets-stl -filename $tmpDir$resultName1

set resultName2 "result2.stl"
save-facets-stl -filename $tmpDir$resultName2 -binary

# Load saved STLs and check meshes.
set nbNodes 336
set nbTriangles 692
set xDim 426.25
set yDim 201.5
set zDim 188.0502
set tolerance 1.0e-4

clear
load-stl $tmpDir$resultName1
test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear
load-stl $tmpDir$resultName2
test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

# Remove temporary files.
file delete -force $tmpDir