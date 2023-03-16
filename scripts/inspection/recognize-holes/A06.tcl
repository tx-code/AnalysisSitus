source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/2lD06EZjpMB6kDapojLsUDk5X2Iq8Zrjz4TlrYUo.stp
set radius 1e10
set refFids { 2 3 4 6 7 8 10 11 12 14 15 16 124 125 126 127 130 131 132 133 136 137 138 139 142 143 144 145 }

__recognize-holes
