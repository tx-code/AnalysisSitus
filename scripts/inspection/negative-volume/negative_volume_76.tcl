set datafile cad/topEdgeFillets/topEdgeFillets_5.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 1448.562397067222 

test-check-number-shape-entities -vertex 6 -edge 9 -wire 5 -face 5 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 30.000000000000036 -yDim 15.000000000000007 -zDim 15 -tol 1.0e-4
