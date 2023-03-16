source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/CNC-milling-7.stp
set radius 1e10
set refFids { 17 18 19 20 21 22 23 24 25 26 27 28 31 32 33 34 35 36 37 38 39 40 63 64 65 66 }

__recognize-holes
