source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0089_tail_wheel_lg_bracket_grabcad.brep
set maxSize   0
set refFids { 60 61 64 69 74 75 114 119 120 121 122 123 124 125 126 127 128 133 193 198 199 200 201 202 203 204 205 206 207 211 }

__recognize-cavities
