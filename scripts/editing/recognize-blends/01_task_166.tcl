set datafile cad/blends/test_task_166.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
load-step $datadir/$datafile
set refBlends { 23 25 27 29 31 34 35 38 }

# Recognize features.
set blends [recognize-blends]

# Check.
set nb [llength $refBlends]
if { [llength $blends] != $nb } {
  return -code error "ERROR: number of blends != $nb"
} else {
  set i 0
  while { $i < $nb } {
    set refBlend [expr {int([lindex $refBlends $i])}]
    set blend [expr {int([lindex $blends $i])}]
    if { $refBlend != $blend } {
      return -code error "ERROR: $refBlend != $blend"
    }
    incr i
  }
}
