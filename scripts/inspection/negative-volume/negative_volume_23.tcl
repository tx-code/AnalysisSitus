set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 38 46

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 790.2378 196.34954 62.5 125.0

test-check-number-shape-entities -vertex 17 -edge 31 -wire 19 -face 19 -shell 4 -solid 4 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 15 -yDim 15 -zDim 15 -tol 1.0e-4
