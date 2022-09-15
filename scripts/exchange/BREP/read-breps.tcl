#------------------------------------------------------------------------------
# This script generates facets for all CAD parts located in
# the specified target directory.
#------------------------------------------------------------------------------

# It is assumed that $workdir is specified.

set filenames []

# Callback on visiting a certain file.
proc on_visit {path} {
  global filenames
#  puts "Next filename: $path"
  lappend filenames $path
}

# Recursive visiting procedure.
proc visit {base glob func} {
  foreach f [glob -nocomplain -types f -directory $base $glob] {
    if {[catch {eval $func [list [file join $base $f]]} err]} {
      puts stderr "error: $err"
    }
  }
  foreach d [glob -nocomplain -types d -directory $base *] {
    visit [file join $base $d] $glob $func
  }
}

# Procedure to find files of a certain type.
proc find_files {base ext} {
  visit $base *.$ext [list on_visit]
}

# Find files with a certain extension.
find_files [lindex $workdir 0] "brep"

# Avoid memory leaks because of collecting undo deltas for meshes
# in the data model (project) of Analysis Situs.
disable-transactions

# Load each model and check.
foreach inFilename $filenames {
  puts "Next model to process: $inFilename"
  if { [catch {load-brep $inFilename -add}] } {
    puts stderr "error: cannot read file."
    continue
  }
}

enable-transactions

puts "Processed [llength $filenames] file(s)."
