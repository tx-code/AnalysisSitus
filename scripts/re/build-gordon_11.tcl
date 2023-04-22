# Set working variables.
set datafile cad/gordon/gordon_test-BeGrid.step
set refErr   0.00001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 3 2 1 -g 4 5 6]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
