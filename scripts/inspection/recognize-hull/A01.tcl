source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/nist/nist_ctc_01.brep
set maxSize   0
set refFids { 5 7 23 25 26 46 47 49 54 55 104 117 }

__recognize-hull
