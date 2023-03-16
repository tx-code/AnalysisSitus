source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/254743.stp
set radius 1e10
set refFids { 8 9 }

__recognize-holes
