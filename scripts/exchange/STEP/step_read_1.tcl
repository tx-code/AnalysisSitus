clear
set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Check model.
set nbVertices 131
set nbEdges 198
set nbWires 117
set nbFaces 87
set nbShells 1
set nbsolids 1
set nbCompsolids 0
set nbCompound 1

set xDim 426.25
set yDim 201.5
set zDim 190.54547
set tolerance 1.0e-4

test-check-number-shape-entities -vertex $nbVertices -edge $nbEdges -wire $nbWires -face $nbFaces -shell $nbShells -solid $nbsolids -compsolid $nbCompsolids -compound $nbCompound
test-check-shape-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear