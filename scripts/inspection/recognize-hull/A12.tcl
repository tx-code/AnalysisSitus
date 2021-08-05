source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/canrec/canrec_001.brep
set maxSize   0
set refFids { 1 3 5 6 9 10 11 12 }

__recognize-hull
