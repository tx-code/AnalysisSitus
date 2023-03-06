source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/199078-PC224900000688_Ultima_BP_160x136_v16.step
set radius 1e10
set refFids { 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 49 50 51 52 53 54 55 56 57 58 59 60 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 }

__recognize-holes
