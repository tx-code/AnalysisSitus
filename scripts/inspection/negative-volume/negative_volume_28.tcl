set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 9 10 11 52 53 54 55 56

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 44049.291584566752 

test-check-number-shape-entities -vertex 20 -edge 30 -wire 12 -face 12 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25 -yDim 40 -zDim 60 -tol 1.0e-4
