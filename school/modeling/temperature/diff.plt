set terminal png size 800, 600
set grid
set output "plot.png"
set title "Difference as a function of dt"
set xlabel "dt"
set ylabel "Difference in Analytical and Numeric Solution"
plot "plot.txt" title "difference" with lines