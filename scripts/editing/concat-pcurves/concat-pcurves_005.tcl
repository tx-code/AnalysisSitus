# CAD file.
set datafile cad/turbines/concat-pcurves/runner1.stp

# Expected cardinal numbers.
set nbVertices 22
set nbEdges 35
set nbWires 15
set nbFaces 15
set nbShells 1
set nbsolids 0
set nbCompsolids 0
set nbCompound 0

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-part $datadir/$datafile
fit
#
if { [check-validity] != 1 } {
  error "Initial part is not valid."
}
#
print-summary
#
set initialToler [get-tolerance]

# Apply geometric operators.
concat-pcurves -face 7 -edges 4 5
concat-pcurves -face 7 -edges 3 4
concat-pcurves -face 7 -edges 2 3
concat-pcurves -face 5 -edges 3 4
concat-pcurves -face 5 -edges 5 6
concat-pcurves -face 5 -edges 4 5
concat-pcurves -face 6 -edges 4 5
concat-pcurves -face 6 -edges 4 5
concat-pcurves -face 6 -edges 4 5
concat-pcurves -face 3 -edges 5 6
concat-pcurves -face 3 -edges 4 5
concat-pcurves -face 3 -edges 3 4

# Check that tolernace has not significantly degraded.
set finalToler [get-tolerance]
puts "Final tolerance ($finalToler) vs initial tolerance ($initialToler)"
#
if { [expr $finalToler - $initialToler] > 1e-3 } {
  error "Significant tolerance degradation (final $finalToler vs initial $initialToler)."
}

# Apply sewing after the tolerance check as sewing will degrade it significantly.
sew -toler $finalToler

# Check contents.
test-check-number-shape-entities -vertex $nbVertices -edge $nbEdges -wire $nbWires -face $nbFaces -shell $nbShells -solid $nbsolids -compsolid $nbCompsolids -compound $nbCompound

# Check validity.
if { [check-validity] != 1 } {
  error "Final part is not valid."
}

# Check contours of faces.
if { [check-contours] != 1 } {
  error "Some faces have open contours."
}
