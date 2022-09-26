set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 58 57 41

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 3836.6597409412457 

test-check-number-shape-entities -vertex 10 -edge 15 -wire 7 -face 7 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25.000000000000092 -yDim 25.000000000000092 -zDim 15 -tol 1.0e-4
