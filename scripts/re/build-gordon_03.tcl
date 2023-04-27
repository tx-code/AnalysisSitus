# Set working variables.
set datafile cad/gordon/test-cim.stp
set refErr   0.066

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 90 89 88 87 86 85 84 83 82 81 80 79 78 77 76 75 74 73 72 71 70 27 -g 4 3]

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
