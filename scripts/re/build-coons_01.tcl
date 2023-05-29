# Set working variables.
set datafile cad/gordon/test_sim_comp.brep
set refErr   0.062

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

make-curve-by-edgeId 1
make-curve-by-edgeId 2
make-curve-by-edgeId 3
make-curve-by-edgeId 4

# Build Gordon surface.
set maxErr [build-coons-linear res curve_1 curve_2 curve_3 curve_4]

puts "Max error: $maxErr"

if { $maxErr > $refErr } {
  return -code error "Max approximation error $maxErr exceeds the max allowed (reference) error $refErr."
}
