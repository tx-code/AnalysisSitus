set datafile cad/topEdgeFillets/topEdgeFillets_8.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 12 1 2 3 4 5 6 7 8 10 15 16

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 174.17175329040646 

test-check-number-shape-entities -vertex 29 -edge 49 -wire 22 -face 21 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 40.000000100000008 -yDim 30.000000100000001 -zDim 10.000000100000003 -tol 1.0e-4
