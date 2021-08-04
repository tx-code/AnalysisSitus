source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/ANC101.brep
set maxSize   0
set refFids { 1 2 7 8 9 85 }

__recognize-hull
