source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0073_isolated_blends_test_25.brep
set maxSize   0
set refFids { 281 283 284 285 286 287 289 294 296 298 }

__recognize-hull
