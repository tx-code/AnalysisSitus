clear

make-box box 1 1 1 1 1 1
set-as-part box
generate-facets -lin 12.727 -ang 5

test-make-triangulation-from-facets

# Make temprary folder.
set subDir "/stl_write_2/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
    file mkdir $tmpDir
}

# Save in new STLs.
set resultName1 "result1.stl"
save-stl -filename $tmpDir$resultName1

set resultName2 "result2.stl"
save-stl -filename $tmpDir$resultName2 -binary

# Load saved STLs and check meshes.
set nbNodes 8
set nbTriangles 12
set xDim 1
set yDim 1
set zDim 1
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