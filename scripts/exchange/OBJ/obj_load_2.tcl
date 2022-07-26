clear
set datafile obj/grabcad_obj2.obj
set datadir $env(ASI_TEST_DATA)
load-obj $datadir/$datafile

# Check mesh.
set nbNodes 3917
set nbTriangles 7680
set xDim 34.684
set yDim 64.643
set zDim 67.4281
set tolerance 1.0e-4

test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear