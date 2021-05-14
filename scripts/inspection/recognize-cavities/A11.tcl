source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/plugs/plug.brep
set maxSize   0
set refFids { 30 36 }

__recognize-cavities
