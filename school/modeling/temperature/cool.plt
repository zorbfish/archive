set terminal png size 800, 600
set grid
set output "cool.png"
set title "Temperature x Time"
set xlabel "Temperature (degrees)"
set ylabel "Time (minutes)"
plot "m.dat" title "Measured (from T2.2)" with lines, \
     "c.dat" title "Computed" with linespoints