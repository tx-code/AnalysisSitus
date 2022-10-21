proc ComparePointsCoordProc { curFilename refFilename tol } {

  # Read points. 
  set fp [open $curFilename r]
  set curData [read $fp]
  close $fp

  set fp [open $refFilename r]
  set refData [read $fp]
  close $fp

  # Normalization.
  set curData [split $curData " \n"]
  set curData [lsearch -all -inline -not -exact $curData {}]

  set refData [split $refData " \n"]
  set refData [lsearch -all -inline -not -exact $refData {}]

  set nb [llength $refData]
  if { [llength $curData] != $nb } {
    return -code error "ERROR: number of points != $nb"
  } else {
    set i 0
    while { $i < $nb } {
      set refCoord [expr {double([lindex $refData $i])}]
      set curCoord [expr {double([lindex $curData $i])}]
      set diff [expr {$refCoord - $curCoord}]
      if {[expr {abs($diff)}] > $tol} {
        return -code error "ERROR: $refCoord != $curCoord"
      }
      incr i
    }
  }

}