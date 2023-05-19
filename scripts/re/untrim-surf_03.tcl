# Set working variables.
set datafile cad/untrim/test.igs
set refErr   7.0

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Untrim surface.
set maxErr [untrim-surf -f 1 -e 6 8 17 18]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
