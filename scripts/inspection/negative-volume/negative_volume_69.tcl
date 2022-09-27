set datafile cad/chamfers/chamfer_7.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1 4 5 6 7

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 192.05719095841738 

test-check-number-shape-entities -vertex 19 -edge 30 -wire 13 -face 13 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 40.000000000000007 -yDim 29.171572875253851 -zDim 10.000000000000011 -tol 1.0e-4
