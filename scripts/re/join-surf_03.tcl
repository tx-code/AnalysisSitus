# Set working variables.
set datafile cad/joinsurf/test-joinsurf_03.stp
set refEnergy 22214.79
set epsilon 0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Join surfaces.
join-surf -f 1 2 -profiles1 3 -profiles2 2 -guides 8 -offset 2 -name resSurf
set energy [get-surface-bending resSurf]

if { [expr abs($energy - $refEnergy)] > $epsilon } {
  return -code error "Surface bending energy $energy is different from the reference energy $refEnergy."
}