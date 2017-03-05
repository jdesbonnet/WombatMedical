
#
# Need to apply filter to the ECG raw data first. 
# octave ./ecg-50hz-filter.m ecgin.dat t.t
# 

set title "ECG sample from TI ADS1292R analog front end IC before/after 50Hz DSP notch filter applied" textcolor rgb 'white'
set xlabel 'Time/sample count (500sps)' textcolor rgb 'white'

set style fill transparent solid 0.3 noborder
set style fill noborder

set style line 1 linecolor rgb "red"
set style line 2 linecolor rgb "blue"


set terminal pngcairo size 1024,600 background rgb 'black'
set output "ecg_chart_filtered.png"
set border lc rgb 'white'
set key tc rgb 'white'

set grid lc rgb 'white'

set label "Joe Desbonnet" at graph -0.05,-0.07 font ",8" tc rgb "white"
set label "http://jdesbonnet.blogspot.com" at graph -0.05,-0.10 font ",8" tc rgb "white"

set grid
set ylabel "L-R voltage (V)" textcolor rgb 'white'
set boxwidth 3600
set style fill solid 1.0
set xrange [461000:462000]
set label "Q" at 461070,0.0378 textcolor rgb 'yellow' font ',32' front
set label "R" at 461110,0.0403 textcolor rgb 'yellow' font ',32' front
set label "S" at 461130,0.0378 textcolor rgb 'yellow' font ',32' front
set label "T" at 461210,0.03885 textcolor rgb 'yellow' font ',32' front
set label "P" at 461470,0.03845 textcolor rgb 'yellow' font ',32' front
plot "< zcat jd-ecg-20131123-1.dat.gz" using 1 with lines lw 1 title "ECG unfiltered", 't.t' using 1 with lines linecolor 'yellow' lw 2 title '50Hz filtered'

