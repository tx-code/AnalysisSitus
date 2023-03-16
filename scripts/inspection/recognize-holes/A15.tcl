source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/t02_part4.stp
set radius 1e10
set refFids { 27 28 29 30 }

__recognize-holes
