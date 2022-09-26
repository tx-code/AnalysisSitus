set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 1 2 3 4 7 8 9 10 11 13 14 15 16 17 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 41 42 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 163230.16247 687.22339

test-check-number-shape-entities -vertex 107 -edge 167 -wire 63 -face 63 -shell 2 -solid 2 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 100.0000000000001 -yDim 100.0000007450581 -zDim 60 -tol 1.0e-4
