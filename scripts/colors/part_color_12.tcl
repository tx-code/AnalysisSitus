clear
set datafile cad/ANC101.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile

# Change color.
set-part-color -color (0,255,0)

# Check color.
check-part-color -color (0,255,0)

load-step $datadir/$datafile

# Check color.
check-face-color -fid 6 -color (0,255,0)
