# Set working variables.
set datafile cad/situ/vexity/vexity_bug1.brep

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-brep $datadir/$datafile
fit

set ref_convex 6
set ref_concave 8
set ref_smooth 4

# Check vexity.
check-vertex-vexity -fid 1 convex concave smooth

# Verify result.
if { $convex != $ref_convex } {
  error "Unexpected number of convex vertices ($ref_convex expected while $convex returned)."
}
if { $concave != $ref_concave } {
  error "Unexpected number of concave vertices ($ref_concave expected while $concave returned)."
}
if { $smooth != $ref_smooth } {
  error "Unexpected number of smooth vertices ($ref_smooth expected while $smooth returned)."
}
