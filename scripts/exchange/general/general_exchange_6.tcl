clear
set datafile mesh/adapter_holder_grabcad-dan-yochelson.stl
set datadir $env(ASI_TEST_DATA)
load-part $datadir/$datafile

# Check mesh.
set nbNodes 6578
set nbTriangles 13164
set xDim 25.27
set yDim 9.525
set zDim 12.7
set tolerance 1.0e-4

test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear