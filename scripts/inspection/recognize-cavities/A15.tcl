source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0053_grabcad_part19.stp
set maxSize   0
set refFids { 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 27 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 48 54 55 56 57 58 59 60 61 63 64 66 68 69 70 71 72 73 74 75 76 77 78 79 80 81 82 83 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 119 122 125 128 129 130 131 132 133 134 135 136 137 138 139 140 141 142 143 144 145 146 147 148 149 150 151 152 153 154 155 156 }

__recognize-cavities