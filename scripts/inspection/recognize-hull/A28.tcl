source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0087_socket_bearing_grabcad.stp
set maxSize   0
set refFids { 4 6 12 13 15 17 20 26 28 29 30 31 32 41 42 43 44 45 }

__recognize-hull
