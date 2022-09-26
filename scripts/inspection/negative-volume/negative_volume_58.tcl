set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 14 31 32 33 34 35 50 59

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 3313.970934564275

test-check-number-shape-entities -vertex 60 -edge 112 -wire 54 -face 54 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 30.000000000000007 -yDim 30.000000745058106 -zDim 30 -tol 1.0e-4
