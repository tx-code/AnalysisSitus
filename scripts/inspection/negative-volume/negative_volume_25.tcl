set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 47

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 20.1189 12937.5 1807.13495 4298.59881

test-check-number-shape-entities -vertex 27 -edge 47 -wire 25 -face 25 -shell 4 -solid 4 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 15.000000000000004 -yDim 90 -zDim 20 -tol 1.0e-4
