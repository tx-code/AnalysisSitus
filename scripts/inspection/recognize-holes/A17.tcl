source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/79890-323907-0074_Sylinterin_tappi_lyhyt.stp
set radius 1e10
set refFids { 1 3 6 7 12 13 14 15 16 17 20 21 }

__recognize-holes
