# Set working variables.
set datafile cad/turbines/Partition_3.step
set refErr   0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Untrim surface.
set maxErr [untrim-surf -f 12 -e 11 20 23 24 -u 7 -v 5]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
