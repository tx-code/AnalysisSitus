source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/as1-oc-214.stp
set maxSize   0
set refFids { 17 20 27 42 57 69 83 85 106 121 136 148 }

__recognize-hull
