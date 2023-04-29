# Set working variables.
set datafile cad/gordon/DT-loft-curves.stp
set refErr   0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 -g 113 65 66 23]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
