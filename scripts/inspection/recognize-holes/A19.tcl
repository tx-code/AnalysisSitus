source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/89786-370722-alumiiniumist_keeraja_korpus.stp
set radius 1e10
set refFids { 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 29 30 31 32 33 40 45 46 47 48 49 50 51 60 61 62 63 64 65 66 67 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 }

__recognize-holes
