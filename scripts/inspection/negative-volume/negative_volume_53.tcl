set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 36 37

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 3062.4999999999995 

test-check-number-shape-entities -vertex 14 -edge 25 -wire 13 -face 13 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 10.000000000000007 -yDim 70 -zDim 5 -tol 1.0e-4
