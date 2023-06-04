# Set working variables.
set datafile cad/joinsurf/test-joinsurf_05.stp
set refEnergy 3716.5
set epsilon 0.1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Join surfaces.
join-surf -f 1 2 -profiles1 6 -profiles2 6 -guides 8 -offset 0.1 -name resSurf
set energy [get-surface-bending resSurf]

if { [expr abs($energy - $refEnergy)] > $epsilon } {
  return -code error "Surface bending energy $energy is different from the reference energy $refEnergy."
}
