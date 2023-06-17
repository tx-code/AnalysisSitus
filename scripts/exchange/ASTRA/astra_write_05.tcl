# Set working variables.
set datafile cad/turbines/Partition_8.brep
set to       cad/astra/astra_write_04.dat
set refNb    8

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Extract surface.
make-surf s9  -fid 9
make-surf s12 -fid 12
make-surf s10 -fid 10
make-surf s8  -fid 8
make-surf s13 -fid 13
make-surf s11 -fid 1

# Save ASTRA file.
save-astra -vars s9 s12 s10 s8 s13 s11 -filename $datadir/$to

# Load ASTRA back.
set nbLoaded [load-astra $datadir/$to]

# Delete temporary file.
file delete -force $datadir/$to

if { $nbLoaded != $refNb } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of saved curves and surfaces."
}

