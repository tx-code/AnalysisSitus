# Set working variables.
set datafile cad/convert-to-bezier/s1.stp
set refErr   0.622
set epsilon  0.001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Make surface.
make-surf s -fid 1

# Convert to Bezier.
set maxErr [convert-to-bezier s -approx]
#
puts "Max conversion error for a surface: $maxErr"

# Validate max deviation.
if { [expr abs($maxErr - $refErr)] > $epsilon } {
  return -code error "Max achieved error is $maxErr while the reference error is $refErr."
}
