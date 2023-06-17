# Set working variables.
set datafile cad/turbines/Partition_8.brep
set to       cad/astra/astra_write_04.dat
set refNb    2

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Extract surface.
make-surf s -fid 13

# Save ASTRA file.
save-astra -vars s -filename $datadir/$to

# Load ASTRA back.
set nbLoaded [load-astra $datadir/$to]

# Delete temporary file.
file delete -force $datadir/$to

if { $nbLoaded != $refNb } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of saved curves and surfaces."
}

