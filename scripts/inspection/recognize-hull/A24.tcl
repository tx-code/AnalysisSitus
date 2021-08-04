source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0077_isolated_blends_test_28.brep
set maxSize   0
set refFids { 97 98 99 100 101 }

__recognize-hull
