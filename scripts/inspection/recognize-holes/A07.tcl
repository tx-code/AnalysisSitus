source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/sheet-metal.step
set radius 1e10
set refFids { 217 221 222 293 297 298 }

__recognize-holes
