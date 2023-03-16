source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/chess-rook-complex.stp
set radius 1e10
set refFids { 1 2 3 8 22 41 60 79 98 117 }

__recognize-holes
