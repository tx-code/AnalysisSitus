set datafile cad/pockets/pockets_3.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 5 6 7 8 9 10 11

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 50215.333473067818

test-check-number-shape-entities -vertex 20 -edge 30 -wire 12 -face 12 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 50.607269477628613 -yDim 116.6969574097588 -zDim 15 -tol 1.0e-4
