# Set working variables.
set datafile cad/turbines/blade.igs

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
concat-pcurves -face 1 -edges 3 4

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
