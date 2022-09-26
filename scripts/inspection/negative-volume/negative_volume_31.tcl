set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 52 53 54 55 56

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 23422.15113 16500 2129.28646 6.43806

test-check-number-shape-entities -vertex 28 -edge 49 -wire 26 -face 26 -shell 4 -solid 4 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 25 -yDim 30 -zDim 60 -tol 1.0e-4
