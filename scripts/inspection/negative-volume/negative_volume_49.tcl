set datafile cad/pockets/pockets_17.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -oneSolid -fids 5 6 7

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 18510.588921886887 

test-check-number-shape-entities -vertex 8 -edge 12 -wire 6 -face 6 -shell 1 -solid 1 -compsolid 0 -compound 0

test-check-shape-aabb-dim -xDim 34.000000000000014 -yDim 29 -zDim 22.421647978720959 -tol 1.0e-4
