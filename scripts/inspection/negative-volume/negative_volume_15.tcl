set datafile cad/pockets/pockets_15.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 151883.11464737996

test-check-number-shape-entities -vertex 10 -edge 15 -wire 7 -face 7 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 150.00000000000014 -yDim 40.000000000000021 -zDim 30.717967697244902 -tol 1.0e-4
