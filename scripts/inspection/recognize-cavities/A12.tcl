source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/canrec/canrec_001.brep
set maxSize   0
set refFids { 7 8 13 14 15 16 }

__recognize-cavities
