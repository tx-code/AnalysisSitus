source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0061_isolated_blends_test_14.brep
set maxSize   0
set refFids { 29 34 35 39 40 48 56 64 65 69 70 78 92 100 101 105 106 114 122 130 131 135 136 141 975 1077 1080 1088 1091 }

__recognize-hull
