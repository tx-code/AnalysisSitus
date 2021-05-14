source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/TESS_TEST.brep
set maxSize   0
set refFids { 1 2 4 9 10 11 12 13 14 113 116 117 118 119 120 121 }

__recognize-cavities
