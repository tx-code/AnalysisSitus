source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0059_isolated_blends_test_13.brep
set maxSize   0
set refFids { 2 3 19 23 26 36 37 41 42 43 55 59 117 120 121 123 126 127 128 130 131 132 133 135 140 146 151 153 154 161 163 164 173 175 176 182 184 185 }

__recognize-hull
