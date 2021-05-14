source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0061_isolated_blends_test_14.brep
set maxSize   0
set refFids { }

__recognize-cavities
