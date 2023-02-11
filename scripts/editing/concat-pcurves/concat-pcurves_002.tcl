# CAD file.
set datafile cad/turbines/blade.igs

# Expected cardinal numbers.
set nbVertices 36
set nbEdges 36
set nbWires 9
set nbFaces 9
set nbShells 0
set nbsolids 0
set nbCompsolids 0
set nbCompound 1

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
concat-pcurves -face 5 -edges 1 2
concat-pcurves -face 5 -edges 1 2
concat-pcurves -face 5 -edges 3 4
concat-pcurves -face 5 -edges 3 4

# Check contents.
test-check-number-shape-entities -vertex $nbVertices -edge $nbEdges -wire $nbWires -face $nbFaces -shell $nbShells -solid $nbsolids -compsolid $nbCompsolids -compound $nbCompound

# Check validity.
if { [check-validity] != 1 } {
  error "Final part is not valid."
}

# Check that tolernace has not significantly degraded.
set finalToler [get-tolerance]
puts "Final tolerance ($finalToler) vs initial tolerance ($initialToler)"
#
if { [expr $finalToler - $initialToler] > 1e-3 } {
  error "Significant tolerance degradation."
}

# Check contours of faces.
if { [check-contours] != 1 } {
  error "Some faces have open contours."
}
