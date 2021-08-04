source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/plugs/plug.brep
set maxSize   0
set refFids { 6 8 31 41 45 }

__recognize-hull
