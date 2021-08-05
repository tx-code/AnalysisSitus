source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0088_spin_secure_grabcad.stp
set maxSize   0
set refFids { 6 10 11 12 13 15 16 17 18 69 86 93 }

__recognize-hull
