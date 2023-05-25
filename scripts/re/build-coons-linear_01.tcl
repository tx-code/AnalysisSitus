# Set working variables.
set datafile cad/coons/001_Compound_4_edges.brep
set refErr   0.062

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Coons surface.
build-coons-linear

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
