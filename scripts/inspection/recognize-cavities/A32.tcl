source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0091_wheel_assembly_part.brep
set maxSize   0
set refFids { 2 3 4 5 6 19 24 25 26 27 28 29 }

__recognize-cavities
