set workdir   "C:/Users/user/Desktop/fra2"
set targetdir $workdir

# Get all STEP files available in the input directory.
set filenames [glob -directory $workdir -- "*.step"]

notifier -init

foreach inFilename $filenames {
  puts "Next file: $inFilename"
  set basename [lindex [file split $inFilename] end]
  set basename [file rootname $basename]

  puts "Target output file: $targetdir/$basename.dxf"

  load-part $inFilename
  orient-part -apply
  #get-dominating-plane nx ny nz

  #puts "Dominating direction is: $nx $ny $nz"

  hlr HLR 0 0 0 0 0 1
  save-dxf -var HLR -filename "$targetdir/$basename-with-hidden-features.dxf"

  notifier -step
}

notifier -finish
