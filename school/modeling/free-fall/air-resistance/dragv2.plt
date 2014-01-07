set terminal png size 800, 600
set title "Free Fall with Drag Force, Fd = C*v^2"
set grid
set xlabel "timestep (t = 0.01)"
set ylabel "vertical displacement (m)"
set output "dragv2.png"
plot "xv2.txt" title "vx = 30m/s" with lines, "x0v2.txt" title "vx = 0m/s" with lines