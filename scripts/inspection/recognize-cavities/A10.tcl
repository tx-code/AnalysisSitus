source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/sheet-metal/azv_clamp.brep
set maxSize   0
set refFids { 7 8 9 10 11 18 }

__recognize-cavities
