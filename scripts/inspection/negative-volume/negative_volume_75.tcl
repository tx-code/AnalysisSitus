set datafile cad/topEdgeFillets/topEdgeFillets_3.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 7242.811985336084 

test-check-number-shape-entities -vertex 6 -edge 9 -wire 5 -face 5 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 15.000000000000043 -yDim 150 -zDim 15.000000000000036 -tol 1.0e-4
