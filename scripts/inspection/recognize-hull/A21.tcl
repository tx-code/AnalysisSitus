source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0088_spin_secure_grabcad.stp
set maxSize   0
set refFids { 2 6 10 11 12 13 15 16 17 18 66 67 69 71 73 74 76 81 82 84 86 88 89 90 91 93 95 97 98 }

__recognize-hull
