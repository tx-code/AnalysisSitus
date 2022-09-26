set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 36 37

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 2937.5 62.5 62.5

test-check-number-shape-entities -vertex 14 -edge 25 -wire 15 -face 15 -shell 3 -solid 3 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 10.000000000000007 -yDim 70 -zDim 5 -tol 1.0e-4
