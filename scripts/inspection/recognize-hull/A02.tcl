source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/nist/nist_ctc_02.brep
set maxSize   100
set refFids { 135 190 200 212 213 219 315 318 337 349 356 362 363 396 420 421 }

__recognize-hull
