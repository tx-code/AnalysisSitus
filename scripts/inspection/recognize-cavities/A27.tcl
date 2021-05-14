source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0085_pulley_protection_system_grabca-frc484.step
set maxSize   0
set refFids { 38 39 40 }

__recognize-cavities
