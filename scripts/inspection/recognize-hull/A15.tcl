source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0053_grabcad_part19.stp
set maxSize   0
set refFids { 26 67 84 127 159 }

__recognize-hull
