clear

make-box box 1 1 1 1 1 1

# Make temprary folder.
set subDir "/brep_write_3/"
set dumpdir $env(ASI_TEST_DUMPING)
set tmpDir $dumpdir$subDir
if { ![file isdirectory $tmpDir] } {
    file mkdir $tmpDir
}

# Save in new BREP.
set resultName "result.brep"
save-brep box $tmpDir$resultName

# Load saved BREP and check shape.
set nbVertices 8
set nbEdges 12
set nbWires 6
set nbFaces 6
set nbShells 1
set nbsolids 1
set nbCompsolids 0
set nbCompound 0

set xDim 1
set yDim 1
set zDim 1
set tolerance 1.0e-4

load-brep $tmpDir$resultName
test-check-number-shape-entities -vertex $nbVertices -edge $nbEdges -wire $nbWires -face $nbFaces -shell $nbShells -solid $nbsolids -compsolid $nbCompsolids -compound $nbCompound
test-check-shape-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

# Remove temporary files.
file delete -force $tmpDir

clear