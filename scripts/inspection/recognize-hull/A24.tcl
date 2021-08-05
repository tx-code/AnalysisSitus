source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0077_isolated_blends_test_28.brep
set maxSize   0
set refFids { 99 }

__recognize-hull
