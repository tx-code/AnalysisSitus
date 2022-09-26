set datafile cad/pockets/pockets_6.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 17 16 15 14 9 10 11 12 8 1 3 2 5 4 7 6 13

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 59400.026369471954 

test-check-number-shape-entities -vertex 20 -edge 36 -wire 18 -face 18 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 50.000000000000028 -yDim 80 -zDim 15.000000000000014 -tol 1.0e-4
