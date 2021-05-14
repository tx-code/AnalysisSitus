source $env(ASI_TEST_SCRIPTS)/inspection/recognize-cavities/__begin

# Set working variables.
set datafile  cad/blends/0090_w-storage_grabcad-rana.step
set maxSize   0
set refFids { }

__recognize-cavities
