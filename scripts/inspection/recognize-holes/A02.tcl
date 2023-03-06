source $env(ASI_TEST_SCRIPTS)/inspection/recognize-holes/__begin

# Set working variables.
set datafile cad/holes/00jZmMUdQKuCsLkMnE3ksmR9Aa28RU5uE59pK1Tc.stp
set radius 1e10
set refFids { 1 2 77 78 }

__recognize-holes
