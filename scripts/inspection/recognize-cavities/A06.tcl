source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/as1-oc-214.stp
set maxSize   0
set refFids { 6 7 14 15 33 34 48 49 63 64 71 72 74 75 76 77 78 79 87 88 89 90 91 92 93 94 95 96 97 98 112 113 127 128 142 143 150 151 153 154 155 156 157 158 }

__recognize-cavities
