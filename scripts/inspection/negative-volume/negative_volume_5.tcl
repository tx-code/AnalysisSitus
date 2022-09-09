set datafile cad/pockets/pockets_5.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 32692.198551418791

test-check-number-shape-entities -vertex 6 -edge 8 -wire 6 -face 4 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 100.0 -yDim 100.0 -zDim 15.000000000000002 -tol 1.0e-4
