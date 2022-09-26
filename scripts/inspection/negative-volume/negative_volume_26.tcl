set datafile cad/pockets/test-shoulder_04.step

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear
load-step $datadir/$datafile

# Compute negative volume
compute-negative-volume -fids 14 31 32 33 34 35 50 59

set-as-part "negativeVolumeShape 1"

test-check-part-shape

test-check-solids-volumes 1.0e-4 163.211369 21.6854 164.892417 122.5 622.94763 22.5 122.5 527.65574 627.5 72 435.88666 1.45428 371.696 0.691741 18 18.8496

test-check-number-shape-entities -vertex 62 -edge 126 -wire 81 -face 81 -shell 16 -solid 16 -compsolid 0 -compound 2

test-check-shape-aabb-dim -xDim 30.000000000000007 -yDim 30.000000745058106 -zDim 30 -tol 1.0e-4
