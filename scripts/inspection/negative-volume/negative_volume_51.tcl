set datafile cad/pockets/pockets_19.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 2

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 88046.852487192722

test-check-number-shape-entities -vertex 8 -edge 12 -wire 6 -face 6 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 150.00000000000009 -yDim 31.29002756211802 -zDim 25.000000000000046 -tol 1.0e-4
