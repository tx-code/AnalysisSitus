set datafile cad/pockets/pockets_2.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 5

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 59999.999999999978 

test-check-number-shape-entities -vertex 8 -edge 12 -wire 6 -face 6 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 50.000000000000355 -yDim 80 -zDim 14.999999999999995 -tol 1.0e-4
