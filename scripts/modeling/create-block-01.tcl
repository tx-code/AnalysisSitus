clear
set Pi 3.1415926535897931
set L0 130
set B0 60
set a1 [expr 20.*$Pi/180.]
set a2 [expr 60.*$Pi/180.]
set V1x [expr -$L0/2 - tan($a1)*$B0/2]
set V1y 0
set V2x [expr $L0/2 + tan($a2)*$B0/2]
set V2y 0
set V3x [expr $L0/2 - tan($a2)*$B0/2]
set V3y $B0
set V4x [expr -$L0/2 + tan($a1)*$B0/2]
set V4y $B0
set d 20.
make-point V1 $V1x $V1y 0.
make-point V2 $V2x $V2y 0.
make-point V3 $V3x $V3y 0.
make-point V4 $V4x $V4y 0.
fit
make-edge e1 -p1 V2 -p2 V1
make-edge e2 -p1 V1 -p2 V4
make-edge e3 -p1 V4 -p2 V3
make-edge e4 -p1 V3 -p2 V2
make-wire w e1 e2 e3 e4
make-face f -w w

set-as-part f
offset-shell -$d -simple -solid

# source C:/Work/analysis_situs/scripts/modeling/create-block-01.tcl
