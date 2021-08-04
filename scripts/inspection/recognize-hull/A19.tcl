source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0090_w-storage_grabcad-rana.step
set maxSize   0
set refFids { 178 179 }

__recognize-hull
