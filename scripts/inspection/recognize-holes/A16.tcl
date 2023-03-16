source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/setbt_f8205080_hem.stp
set radius 1e10
set refFids { 122 123 124 125 278 279 280 281 282 283 284 285 286 287 288 289 290 291 292 293 }

__recognize-holes
