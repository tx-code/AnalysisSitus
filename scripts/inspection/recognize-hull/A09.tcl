source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/isolated-heuristics-test.stp
set maxSize   0
set refFids { 3 4 5 7 8 9 }

__recognize-hull
