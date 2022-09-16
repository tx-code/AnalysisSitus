set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 7 8 14 31 32 33 34 35 36 37 38 39 41 46 47 50 57 58 59

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 56053.050675241524 

test-check-number-shape-entities -vertex 67 -edge 107 -wire 42 -face 42 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 30.00000000000005 -yDim 100.0000007450581 -zDim 50 -tol 1.0e-4
