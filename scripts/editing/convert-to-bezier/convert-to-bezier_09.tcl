# Set working variables.
set datafile cad/convert-to-bezier/s3.stp
set refErr   1e-7
set epsilon  0.0001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Make surface.
make-surf s -fid 4

# Convert to Bezier.
set maxErr [convert-to-bezier s]
#
puts "Max conversion error for a surface: $maxErr"

# Validate max deviation.
if { [expr abs($maxErr - $refErr)] > $epsilon } {
  return -code error "Max achieved error is $maxErr while the reference error is $refErr."
}
