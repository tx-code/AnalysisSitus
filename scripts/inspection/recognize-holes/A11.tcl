source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/260953.stp
set radius 1e10
set refFids { 1 2 3 5 6 7 20 21 35 36 37 38 39 40 41 }

__recognize-holes
