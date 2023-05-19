# Set working variables.
set datafile cad/turbines/Partition_3.step
set refErr   0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Untrim surface.
set maxErr [untrim-surf -f 18 -e 23 33 35 36 -u 10 -v 10]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
