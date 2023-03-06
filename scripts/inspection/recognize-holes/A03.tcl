source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/obstructed-hole.step
set radius 1e10
set refFids { 5 7 }

__recognize-holes
