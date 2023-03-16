source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/blends/0027_customblend_03.brep
set radius 1e10
set refFids { 19 20 21 22 23 24 25 26 35 36 37 38 39 40 41 42 48 49 50 51 52 53 }

__recognize-holes
