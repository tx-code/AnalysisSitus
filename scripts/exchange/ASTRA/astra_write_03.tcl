clear
set to   cad/astra/surfaceOfRev_test.dat
set from cad/astra/surfaceOfRev.dat

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
set nbLoaded [load-astra $datadir/$from]

if { $nbLoaded != 2 } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of loaded curves and surfaces."
}

save-astra -vars st PVL PST -filename $datadir/$to

set nbLoaded [load-astra $datadir/$to]

if { $nbLoaded != 3 } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of saved curves and surfaces."
}

file delete -force $datadir/$to

clear