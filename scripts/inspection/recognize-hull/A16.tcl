source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0055_isolated_blends_test_10.brep
set maxSize   0
set refFids { 1 13 }

__recognize-hull
