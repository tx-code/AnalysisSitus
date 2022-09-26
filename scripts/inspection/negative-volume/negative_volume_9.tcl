set datafile cad/pockets/pockets_9.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 5 6

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 74942.057504117314

test-check-number-shape-entities -vertex 12 -edge 18 -wire 8 -face 8 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 50.000000000000142 -yDim 100.00000000000004 -zDim 15.000000000000002 -tol 1.0e-4
