source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/TESS_TEST.brep
set maxSize   0
set refFids { 3 5 6 32 33 37 38 42 43 47 48 52 53 57 58 62 63 67 68 72 73 77 78 82 83 87 88 92 93 97 98 102 103 107 108 112 }

__recognize-hull
