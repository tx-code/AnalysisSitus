set datafile cad/pockets/pockets_4.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 5 6 7

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 44860.290565697163 

test-check-number-shape-entities -vertex 12 -edge 18 -wire 8 -face 8 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 58.875900129637344 -yDim 95.459833549395597 -zDim 14.999999999999991 -tol 1.0e-4
