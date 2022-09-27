set datafile cad/chamfers/chamfer_7.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 4 5 6 7 9 10 12

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 359.24040804255048 

test-check-number-shape-entities -vertex 25 -edge 39 -wire 16 -face 16 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 40.000000000000007 -yDim 30.000000000000007 -zDim 10.000000000000011 -tol 1.0e-4
