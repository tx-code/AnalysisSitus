source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0073_isolated_blends_test_25.brep
set maxSize   0
set refFids { 1 2 3 4 5 81 82 83 84 85 86 87 88 89 90 91 92 93 94 95 96 97 98 99 100 101 102 103 104 105 106 107 108 109 110 111 112 113 114 115 116 117 118 119 120 121 122 123 124 125 126 127 128 129 130 131 132 133 134 135 136 137 138 256 257 258 259 260 261 262 263 264 265 266 267 }

__recognize-cavities
