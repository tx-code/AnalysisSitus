clear
set datafile cad/astra/lna.DAT
set refNb 2

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
set nbLoaded [load-astra $datadir/$datafile]

if { $nbLoaded != $refNb } {
  puts "...      error: Expected $refNb entities while loaded $nbLoaded."
  error "Unexpected number of loaded curves and surfaces."
}
