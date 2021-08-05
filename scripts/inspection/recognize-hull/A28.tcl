source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0087_socket_bearing_grabcad.stp
set maxSize   0
set refFids { 12 13 14 18 20 26 42 43 }

__recognize-hull
