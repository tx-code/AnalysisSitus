source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0088_spin_secure_grabcad.stp
set maxSize   0
set refFids { 2 7 10 12 13 15 16 17 18 66 69 73 74 76 81 82 86 89 90 93 97 98 }

__recognize-hull
