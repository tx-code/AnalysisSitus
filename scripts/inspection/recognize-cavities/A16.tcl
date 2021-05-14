source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0055_isolated_blends_test_10.brep
set maxSize   0
set refFids { 2 3 4 5 6 7 8 9 10 11 12 44 45 46 47 48 49 50 51 52 113 114 115 116 }

__recognize-cavities
