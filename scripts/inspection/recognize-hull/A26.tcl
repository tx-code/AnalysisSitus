source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0082_MVE_100-3_AP203.stp
set maxSize   0
set refFids { 109 367 369 383 387 508 525 550 551 }

__recognize-hull
