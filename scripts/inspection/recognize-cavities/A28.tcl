source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0087_socket_bearing_grabcad.stp
set maxSize   0
set refFids { 1 2 3 7 9 10 11 21 22 23 27 47 48 49 }

__recognize-cavities
