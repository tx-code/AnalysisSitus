set datafile cad/pockets/pockets_12.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 2 3 4

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 42913.176683626953 

test-check-number-shape-entities -vertex 8 -edge 12 -wire 6 -face 6 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 47.017606291134499 -yDim 118.18551559741442 -zDim 15.000000000000007 -tol 1.0e-4
