source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/nist/nist_ctc_05.brep
set maxSize   0
set refFids { 102 152 153 165 }

__recognize-hull
