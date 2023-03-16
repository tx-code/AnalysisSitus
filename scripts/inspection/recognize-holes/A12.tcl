source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/277345.stp
set radius 1e10
set refFids { 1 53 54 58 59 60 61 62 63 64 65 66 97 }

__recognize-holes
