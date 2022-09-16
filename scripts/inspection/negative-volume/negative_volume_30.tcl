set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 7 8 13 14 15 16 17 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 41 42 46 47 48 49 50 51 57 58 59 60 61 62

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 118623.46033627579 

test-check-number-shape-entities -vertex 114 -edge 182 -wire 70 -face 70 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 100.0000000000001 -yDim 100.0000007450581 -zDim 50 -tol 1.0e-4
