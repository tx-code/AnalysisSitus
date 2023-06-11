# Set working variables.
set datafile cad/convert-to-c2/from-narva_01.stp
set refErr   0.006
set epsilon  0.001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Make curve.
make-curve c -eid 1

# Convert to C2.
set maxErr [convert-to-c2 c]
puts "Max conversion error: $maxErr"

# Validate max deviation.
if { [expr abs($maxErr - $refErr)] > $epsilon } {
  return -code error "Max achieved error is $maxErr while the reference error is $refErr."
}
