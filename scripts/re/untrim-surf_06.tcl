# Set working variables.
set datafile cad/turbines/Partition_3.step
set refErr   0.001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Untrim surface.
set maxErr [untrim-surf -f 13 -e 15 26 27 28 -u 10 -v 10]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
