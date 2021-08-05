source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0079_level_grabcad-rana.step
set maxSize   0
set refFids { 1 4 12 15 22 37 }

__recognize-hull
