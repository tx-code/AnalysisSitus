source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/Mill_2_NOCUT_1_490-025C5-08M.stp
set radius 1e10
set refFids { 6 7 20 29 30 31 87 142 197 }

__recognize-holes
