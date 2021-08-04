source $env(ASI_TEST_SCRIPTS)/inspection/recognize-hull/__begin

# Set working variables.
set datafile  cad/blends/0085_pulley_protection_system_grabca-frc484.step
set maxSize   0
set refFids { 5 7 8 9 10 11 12 13 14 15 16 17 18 19 20 23 24 34 36 }

__recognize-hull
