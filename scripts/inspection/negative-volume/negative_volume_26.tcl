set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 14 31 32 33 34 35 50 59

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 2799.4710268975687 

test-check-number-shape-entities -vertex 44 -edge 74 -wire 32 -face 32 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 30.000000000000007 -yDim 30.000000745058106 -zDim 30 -tol 1.0e-4
