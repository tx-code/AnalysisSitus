set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 15 16

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 1136.1720055569408

test-check-number-shape-entities -vertex 32 -edge 58 -wire 28 -face 28 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 43.999998718500208 -yDim 7.0000001043081994 -zDim 4 -tol 1.0e-4
