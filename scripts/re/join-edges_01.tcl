# Set working variables.
set datafile cad/gordon/loft2.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Concat curves
concat-curves-and-check -edges 20 9
