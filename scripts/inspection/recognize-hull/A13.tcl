source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0031_grabcad_crankshaft_pavan-patil.brep
set maxSize   0
set refFids { 32 181 }

__recognize-hull
