clear
set to   cad/astra/surface_test.dat
set from cad/astra/surface.dat
set refNb 1

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
set nbLoaded [load-astra $datadir/$from]

if { $nbLoaded != $refNb } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of loaded curves and surfaces."
}

save-astra -vars PVL -filename $datadir/$to

set nbLoaded [load-astra $datadir/$to]

if { $nbLoaded != $refNb } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of saved curves and surfaces."
}

file delete -force $datadir/$to

clear