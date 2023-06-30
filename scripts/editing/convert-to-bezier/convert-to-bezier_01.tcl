# Set working variables.
set datafile cad/convert-to-bezier/test.igs
set refErrS  1e-7
set refErrC  1e-7
set epsilon  0.001

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Make surface.
make-surf s -fid 1

# Make edge.
make-curve c -eid 5

# Convert to Bezier.
set maxErrS [convert-to-bezier s]
set maxErrC [convert-to-bezier c]
#
puts "Max conversion error for a surface: $maxErrS"
puts "Max conversion error for a curve: $maxErrC"

# Validate max deviation.
if { [expr abs($maxErrS - $refErrS)] > $epsilon } {
  return -code error "Max achieved error is $maxErrS while the reference error is $refErrS."
}
#
if { [expr abs($maxErrC - $refErrC)] > $epsilon } {
  return -code error "Max achieved error is $maxErrC while the reference error is $refErrC."
}
