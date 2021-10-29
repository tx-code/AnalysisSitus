set workdir   C:/work/Extensions/AnalysisSitus_extensions/data/canrec/sheet-metal/98208
set targetdir C:/work/Extensions/AnalysisSitus_extensions/data/canrec/sheet-metal/98208/98208_out

# Get all STEP files available in the input directory.
set filenames [glob -directory $workdir -- "*.stp"]

notifier -init

foreach inFilename $filenames {
  puts "Next file: $inFilename"
  set basename [lindex [file split $inFilename] end]
  set basename [file rootname $basename]

  puts "Target output file: $targetdir/$basename.stp"
  load-step $inFilename
  convert-to-canonical
  save-step "$targetdir/$basename.stp"

  notifier -step
}

notifier -finish
