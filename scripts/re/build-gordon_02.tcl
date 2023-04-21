# Set working variables.
set datafile cad/gordon/test-cim.stp
set refErr   0.045

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 92 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 91 -g 4 1]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
