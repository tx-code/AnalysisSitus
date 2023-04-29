# Set working variables.
set datafile cad/gordon/DT-loft-curves.stp
set refErr   0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 112 111 110 109 108 107 106 105 104 103 102 101 100 99 98 97 96 95 94 93 92 91 -g 113 114 115 116]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
