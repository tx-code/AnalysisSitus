set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 15 16

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 50 0.0860579 1019.99989 7.99999 50 7.99999 0.0860579

test-check-number-shape-entities -vertex 32 -edge 60 -wire 36 -face 36 -shell 7 -solid 7 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 43.999998718500208 -yDim 7.0000001043081994 -zDim 4 -tol 1.0e-4
