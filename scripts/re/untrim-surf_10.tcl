# Set working variables.
set datafile cad/untrim/00261_TES1.BREP
set refErr   0.001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Untrim surface.
set maxErr [untrim-surf -f 1 -e 1 2 3 4 -u 10 -v 10]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
