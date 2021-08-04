source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0073_isolated_blends_test_25.brep
set maxSize   0
set refFids { 280 281 282 283 284 285 286 287 288 289 293 294 295 296 298 }

__recognize-hull
