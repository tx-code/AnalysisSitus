source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0071_isolated_blends_test_23.brep
set maxSize   0
set refFids { 33 77 78 79 80 81 82 83 84 85 99 112 150 }

__recognize-hull
