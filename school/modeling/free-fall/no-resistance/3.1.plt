set terminal png size 800, 600
set title "Styrofoam ball in Free Fall (with no drag)"
set grid
set xlabel "timestep (t = 0.01)"
set ylabel ""
set output "3.1.png"
plot "y.txt" title "position (m)" with lines, "v.txt" title "velocity (m/s)" with lines, "a.txt" title "acceleration (m/s^2)" with lines