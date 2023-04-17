# Set working variables.
set datafile cad/gordon/test-cim.stp

# Read input geometry.
set datadir $env(ASI_TEST_DATA)
clear; load-part $datadir/$datafile; fit
fit

# Build Gordon surfaces.
build-gordon -name s1 -p 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 -g 2 3
build-gordon -name s2 -p 92 48 47 46 45 44 43 42 41 40 39 38 37 36 35 34 33 32 31 30 29 91 -g 4 1
build-gordon -name s3 -p 90 89 88 87 86 85 84 83 82 81 80 79 78 77 76 75 74 73 72 71 70 27 -g 4 3
build-gordon -name s4 -p 69 68 67 66 65 64 63 62 61 60 59 58 57 56 55 54 53 52 51 50 49 28 -g 1 2

# Make faces.
make-face f1 s1
make-face f2 s2
make-face f3 s3
make-face f4 s4

# Put faces into a shell.
make-shell shell f1 f2 f3 f4

# Set as an active part.
set-as-part shell

# Stitch faces.
sew -toler 0.1

# Check validity.
if { [check-validity] != 1 } {
  error "Final part is not valid."
}

# Check contours of faces.
if { [check-contours] != 1 } {
  error "Some faces have open contours."
}
