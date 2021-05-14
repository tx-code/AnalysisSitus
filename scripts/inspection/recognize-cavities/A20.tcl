source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0068_isolated_blends_test_20.brep
set maxSize   100
set refFids { 7 8 9 10 11 12 13 14 15 35 }

__recognize-cavities
