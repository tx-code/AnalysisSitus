# Set working variables.
set datafile     cad/approx-contour/approx-contour_04.brep
set nbVertices   8
set nbEdges      8
set nbWires      1
set nbFaces      0
set nbShells     0
set nbSolids     0
set nbCompsolids 0
set nbCompound   0

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

print-summary

# Approximate contour.
re-approx-contour -apply

print-summary

# Check model.
test-check-number-shape-entities -vertex $nbVertices -edge $nbEdges -wire $nbWires -face $nbFaces -shell $nbShells -solid $nbSolids -compsolid $nbCompsolids -compound $nbCompound
