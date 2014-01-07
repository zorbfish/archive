set terminal png size 800, 600
set title "Free Fall with Drag Force and Terminal Velocity"
set grid
set xlabel "timestep (t = 0.01)"
set ylabel "vertical displacement (m)"
set output "dragf.png"
plot "tv.txt" title "linear" with lines, "tv2.txt" title "quadratic" with lines, "emp.txt" title "empirical" with lines