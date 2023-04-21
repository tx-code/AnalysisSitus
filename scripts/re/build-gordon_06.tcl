# Set working variables.
set datafile cad/gordon/test-cim.stp
set refErr   0.069

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 28 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 -g 1 2]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
