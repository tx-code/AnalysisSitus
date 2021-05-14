source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0059_isolated_blends_test_13.brep
set maxSize   0
set refFids { 34 35 215 216 217 218 219 221 222 223 224 }

__recognize-cavities
