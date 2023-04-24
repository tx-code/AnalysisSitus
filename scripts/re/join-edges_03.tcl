# Set working variables.
set datafile cad/gordon/loft1.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Concat curves
concat-curves-and-check -edges 23 22
concat-curves-and-check -edges 22 21