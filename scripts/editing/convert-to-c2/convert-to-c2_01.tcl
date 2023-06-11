# Set working variables.
set datafile cad/convert-to-c2/sr-c1.igs
set refErr   0.485
set epsilon  0.001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Make surface.
make-surf s -fid 1

# Convert to C2.
set maxErr [convert-to-c2 s]
puts "Max conversion error: $maxErr"

# Validate max deviation.
if { [expr abs($maxErr - $refErr)] > $epsilon } {
  return -code error "Max achieved error is $maxErr while the reference error is $refErr."
}
