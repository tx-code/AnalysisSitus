set datafile cad/topEdgeFillets/topEdgeFillets_7.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 2 3 4 5

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 39.207429618317768 

test-check-number-shape-entities -vertex 12 -edge 25 -wire 11 -face 11 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 25.450256216681559 -yDim 5.6491791349901881 -zDim 2.7154244483501202 -tol 1.0e-4
