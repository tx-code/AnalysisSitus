source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0048_cable_tie_slot_10_v6_colored.step
set maxSize   0
set refFids { 2 18 19 71 72 73 74 96 97 98 99 100 105 111 112 113 114 115 116 177 178 232 237 238 239 240 241 242 243 244 245 246 247 248 250 251 255 259 }

__recognize-hull
