set datafile cad/chamfers/chamfer_6.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 1

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 179.59438003021671 

test-check-number-shape-entities -vertex 4 -edge 6 -wire 4 -face 3 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 14 -yDim 14 -zDim 3.5000000000000071 -tol 1.0e-4
