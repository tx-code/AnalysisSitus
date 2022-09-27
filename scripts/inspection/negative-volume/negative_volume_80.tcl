set datafile cad/topEdgeFillets/topEdgeFillets_7.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 14 15 16 17 9 10 11 1 2 3 4 5

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 331.21496033426894 

test-check-number-shape-entities -vertex 28 -edge 51 -wire 22 -face 21 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 40.000000000000014 -yDim 30 -zDim 10.000000099999998 -tol 1.0e-4
