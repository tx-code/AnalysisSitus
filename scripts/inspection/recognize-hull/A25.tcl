source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0079_level_grabcad-rana.step
set maxSize   0
set refFids { 1 2 4 6 12 13 15 17 19 22 32 33 34 37 49 56 62 69 }

__recognize-hull
