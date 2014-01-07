set terminal png size 800, 600
set grid
set output "rms.png"
set title "Error in cooling constant r"
set xlabel "r"
set ylabel "Root Mean Square Error"
plot "rms.dat" title "RMS" with lines