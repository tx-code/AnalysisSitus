set datafile cad/pockets/pockets_13.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 2 3 4

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 111218.11434599504 

test-check-number-shape-entities -vertex 12 -edge 18 -wire 8 -face 8 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 130.00000000000006 -yDim 200 -zDim 15.000000000000021 -tol 1.0e-4
