source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0088_spin_secure_grabcad.stp
set maxSize   0
set refFids { 3 4 64 65 }

__recognize-cavities
