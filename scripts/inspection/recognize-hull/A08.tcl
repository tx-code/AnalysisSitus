source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0091_wheel_assembly_part.brep
set maxSize   0
set refFids { 1 10 11 12 13 15 17 18 47 48 49 57 58 59 67 68 70 71 72 74 }

__recognize-hull
