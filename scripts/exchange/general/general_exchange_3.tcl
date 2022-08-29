clear
set datafile cad/iges/grabcad_iges_1.iges

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-part $datadir/$datafile

# Check model.
set nbVertices 991
set nbEdges 1696
set nbWires 713
set nbFaces 710
set nbShells 1
set nbsolids 1
set nbCompsolids 0
set nbCompound 0

set xDim 32.6227
set yDim 32.4691
set zDim 298.45
set tolerance 1.0e-4

test-check-number-shape-entities -vertex $nbVertices -edge $nbEdges -wire $nbWires -face $nbFaces -shell $nbShells -solid $nbsolids -compsolid $nbCompsolids -compound $nbCompound
test-check-shape-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear