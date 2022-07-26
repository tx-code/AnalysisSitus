clear
set datafile ply/ANC101.ply
set datadir $env(ASI_TEST_DATA)
load-ply $datadir/$datafile

# Check mesh.
set nbNodes 12608
set nbTriangles 25236
set xDim 0.42625
set yDim 0.2015
set zDim 0.190545
set tolerance 1.0e-4

test-check-number-mesh-entities -nodes $nbNodes -tri $nbTriangles
test-check-mesh-aabb-dim -xDim $xDim -yDim $yDim -zDim $zDim -tol $tolerance

clear