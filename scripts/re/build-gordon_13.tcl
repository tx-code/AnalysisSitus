# Set working variables.
set datafile cad/gordon/DT-loft-curves.stp
set refErr   0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 22 21 20 19 18 17 16 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 -g 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
