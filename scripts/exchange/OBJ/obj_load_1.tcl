clear
set datafile obj/grabcad_obj1.obj
set datadir $env(ASI_TEST_DATA)
load-obj $datadir/$datafile

# Check mesh.
set nbNodes 178
set nbTriangles 176
set xDim 0.378015
set yDim 0.0195932
set zDim 0.379154
set tolerance 1.0e-4

test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear