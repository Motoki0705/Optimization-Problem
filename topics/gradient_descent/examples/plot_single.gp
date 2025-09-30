# examples/plot_single.gp
# Variables to be provided with -e:
#   CSV  : 'path/to/file.csv'       (string)
#   PNG  : 'path/to/file.png'       (string)
#   A3,A2,A1,A0 : coefficients      (numbers)
#   TITLE : custom title (string)   (optional)

set terminal pngcairo size 1000,700 enhanced font 'Arial,12'
set output PNG
set datafile separator ","
set key outside right top autotitle columnhead
set grid
set xlabel "x"
set ylabel "f(x)"

if (!exists("TITLE")) TITLE = "Gradient Descent trace"
set title TITLE

# f(x)
f(x) = A3*x**3 + A2*x**2 + A1*x + A0

# Autoscale per-CSV
set autoscale

# CSV format: iter(1), value(2), grad(3), lr(4), x(5)
# Plot the function curve and the trajectory (x, f(x)) from CSV
plot f(x) with lines lw 2 title sprintf("f(x) = %.3gx^3%+.3gx^2%+.3gx%+.3g", A3,A2,A1,A0), \
     CSV using 5:2 with linespoints pt 7 ps 1.1 title "trajectory"

