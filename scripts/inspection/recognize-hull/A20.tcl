source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0068_isolated_blends_test_20.brep
set maxSize   100
set refFids { 3 4 5 6 16 17 19 24 27 30 34 36 37 52 }

__recognize-hull
