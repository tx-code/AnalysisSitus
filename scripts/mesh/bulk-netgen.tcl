#------------------------------------------------------------------------------
# This script generates facets for all CAD parts located in
# the specified target directory.
#------------------------------------------------------------------------------

set workdir "C:/work/Extensions/AnalysisSitus_extensions/data/dfm"
set targetExt "stp"

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
find_files [lindex $workdir 0] $targetExt

# Load each model and check.
foreach inFilename $filenames {
  puts "Next model to process: $inFilename"
  clear
  if { [catch {load-step $inFilename}] } {
    puts stderr "error: cannot read a STEP file."
    continue
  }
  
  poly-netgen
}

puts "Checked [llength $filenames] files."
