source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0082_MVE_100-3_AP203.stp
set maxSize   0
set refFids { 33 34 109 367 369 370 371 383 387 508 523 524 525 526 527 528 529 530 550 551 }

__recognize-hull
