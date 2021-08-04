source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/sheet-metal/azv_clamp.brep
set maxSize   0
set refFids { 1 2 3 5 12 14 15 16 17 }

__recognize-hull
