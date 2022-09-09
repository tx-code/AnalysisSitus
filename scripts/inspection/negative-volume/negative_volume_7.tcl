set datafile cad/pockets/pockets_7.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 5 6 7 8

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 139729.6016858808 

test-check-number-shape-entities -vertex 16 -edge 24 -wire 10 -face 10 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 50.000000000000028 -yDim 80 -zDim 35.000000000000028 -tol 1.0e-4
