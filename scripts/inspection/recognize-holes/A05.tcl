source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/hEWvKLBpb9TXx22l7U6hkCbUOGWrl3cuSgPlkz38.stp
set radius 1e10
set refFids { 2 3 5 6 7 8 9 10 11 12 13 14 16 64 65 66 67 68 69 70 71 72 73 74 75 100 101 102 103 104 105 106 107 }

__recognize-holes
