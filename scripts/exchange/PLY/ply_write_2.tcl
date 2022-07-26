clear

make-box box 1 1 1 1 1 1
set-as-part box
generate-facets -lin 12.727 -ang 5

test-make-triangulation-from-facets

# Make temprary folder.
set subDir "/ply_write_2/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
    file mkdir $tmpDir
}

# Save in new PLY.
set resultName "result.ply"
save-ply $tmpDir$resultName

# Load saved STLs and check meshes.
set nbNodes 8
set nbTriangles 12
set xDim 1
set yDim 1
set zDim 1
set tolerance 1.0e-4

clear
load-ply $tmpDir$resultName
test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

# Remove temporary files.
clear
file delete -force $tmpDir