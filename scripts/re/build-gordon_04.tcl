# Set working variables.
set datafile cad/gordon/test-cim.stp
set refErr   0.069

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surface.
set maxErr [build-gordon -p 69 68 67 66 65 64 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 28 -g 1 2]

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
