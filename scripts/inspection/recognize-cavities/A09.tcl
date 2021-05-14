source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/isolated-heuristics-test.stp
set maxSize   0
set refFids { 6 10 11 12 13 14 15 }

__recognize-cavities
