# Set working variables.
set datafile cad/convert-to-c2/sr-c1.igs
set refEnergy 940.144
set refErr    0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Convert surface to C2.
puts "Max conversion error: [convert-surf-c2 s -add-knots]"
set energy [get-surface-bending resSurf]

if { [expr abs($energy - $refEnergy)] > $epsilon } {
  return -code error "Surface bending energy $energy is different from the reference energy $refEnergy."
}
