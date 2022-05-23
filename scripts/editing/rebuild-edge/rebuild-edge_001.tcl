# Set working variables.
set datafile cad/box.brep

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-brep $datadir/$datafile
fit
#
if { [check-validity] != 1 } {
  error "Initial part is not valid."
}
#
print-summary
#
set initialToler [get-tolerance]

# Add some metadata.
set-face-color -fid 1 -color rgb(255,225,50)
set-face-color -fid 2 -color rgb(255,125,50)
set-face-color -fid 3 -color rgb(215,225,150)

# Initialize naming service
init-naming

# Apply geometric operators.
rebuild-edge -name edge_1

# Check metadata.
check-part-metadata

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

# Check orientations of vertices.
if { [check-vertices-ori] != 1 } {
  error "Some edges have non-distinguishable orientations of vertices."
}

# Check contours of faces.
if { [check-contours] != 1 } {
  error "Some faces have open contours."
}
