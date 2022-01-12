# Set working variables.
set datafile cad/situ/vexity/cable-inlet-with-threads.stp

# -1: undefined
#  0: concave
#  1: convex
#  2: smooth
#  3: smooth concave
#  4: smooth convex
set ref_vexity 3

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-part $datadir/$datafile
fit

# Check vexity.
set vexity [check-edge-vexity 257]

# Verify result.
if { $vexity != $ref_vexity } {
  error "Unexpected vexity ($ref_vexity expected while $vexity returned)."
}
