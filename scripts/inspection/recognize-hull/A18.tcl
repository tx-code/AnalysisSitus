source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0061_isolated_blends_test_14.brep
set maxSize   0
set refFids { 28 29 30 33 34 35 39 40 41 47 48 49 55 56 57 63 64 65 69 70 71 77 78 79 91 92 93 99 100 101 105 106 107 113 114 115 121 122 123 129 130 131 135 136 137 140 141 142 975 1063 1071 1076 1077 1080 1087 1088 1091 }

__recognize-hull
