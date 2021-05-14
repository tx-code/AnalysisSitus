source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/TESS_TEST.brep
set maxSize   20
set refFids { 1 2 118 119 120 121 }

__recognize-cavities
