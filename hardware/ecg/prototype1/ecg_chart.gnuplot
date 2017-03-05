
set title "ECG sample from ADS1292R prototype\nNote strong 50Hz mains interference." textcolor rgb 'white'
set xlabel 'Time/sample count (500sps)' textcolor rgb 'white'

set style fill transparent solid 0.3 noborder
set style fill noborder

set style line 1 linecolor rgb "red"
set style line 2 linecolor rgb "blue"


set terminal pngcairo size 1024,600 background rgb 'black'
set output "ecg_chart.png"
set border lc rgb 'white'
set key tc rgb 'white'

set grid lc rgb 'white'

set label "Joe Desbonnet" at graph -0.05,-0.07 font ",8" tc rgb "white"
set label "http://jdesbonnet.blogspot.com" at graph -0.05,-0.10 font ",8" tc rgb "white"

#set xdata time
#set timefmt "%Y%m%d-%H%M%S"
set grid
#set format x "%Y\n%m/%d"
#set format x "%d %b"
set ylabel "Voltage (V)" textcolor rgb 'white'
set boxwidth 3600
set style fill solid 1.0
set xrange [461000:462000]
set label "Q" at 461020,0.0372 textcolor rgb 'white'
set label "R" at 461050,0.0407 textcolor rgb 'white'
set label "S" at 461060,0.0372 textcolor rgb 'white'
set label "T" at 461150,0.0395 textcolor rgb 'white'
set label "P" at 461400,0.0390 textcolor rgb 'white'
plot "< zcat jd-ecg-20131123-1.dat.gz" using 1 with lines lw 1 title "ECG"


