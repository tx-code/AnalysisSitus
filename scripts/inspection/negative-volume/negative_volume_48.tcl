set datafile cad/pockets/pockets_16.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 2

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 152153.90309173457

test-check-number-shape-entities -vertex 8 -edge 12 -wire 6 -face 6 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 150.00000000000014 -yDim 40.000000000000021 -zDim 30.717967697244902 -tol 1.0e-4
