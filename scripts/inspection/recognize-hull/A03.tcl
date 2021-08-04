source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/nist/nist_ctc_04_colored.stp
set maxSize   100
set refFids { 234 235 238 239 243 382 483 }

__recognize-hull
