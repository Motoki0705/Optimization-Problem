#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") -a <a3> -b <a2> -c <a1> -d <a0> -i <csv> -o <output>

Options:
  -a, --a3 VALUE        Cubic coefficient for x^3 term (required)
  -b, --a2 VALUE        Cubic coefficient for x^2 term (required)
  -c, --a1 VALUE        Cubic coefficient for x term (required)
  -d, --a0 VALUE        Constant term (required)
  -i, --input FILE      Gradient descent CSV trace (required)
  -o, --output FILE     Output image path (required)
      --format fmt      Output format: png (default) or svg
      --margin FACTOR   Margin factor beyond extrema/trajectory (default 1.5)
      --point-size VAL  Point size for trajectory markers (default 1.1)
      --line-width VAL  Line width for trajectory polyline (default 1.5)
      --curve-width VAL Line width for f(x) curve (default 2)
      --traj-color HEX  Colour for trajectory (default #d62728)
      --curve-color HEX Colour for objective curve (default #1f77b4)
      --xrange MIN:MAX  Override automatic x-range
      --title TEXT      Custom plot title
      --show            Attempt to open the output with xdg-open when finished
      --help            Show this message
USAGE
}

require_cmd() {
  if ! command -v "$1" >/dev/null 2>&1; then
    echo "Error: required command '$1' not found" >&2
    exit 1
  fi
}

require_cmd gnuplot
require_cmd mktemp
require_cmd awk
require_cmd dirname
require_cmd sort

format="png"
margin_factor="1.5"
point_size="1.1"
line_width="1.5"
curve_width="2"
traj_color="#d62728"
curve_color="#1f77b4"
xrange_override=""
custom_title=""
show_flag=0

a3=""
a2=""
a1=""
a0=""
csv=""
output=""

while [ $# -gt 0 ]; do
  case "$1" in
    -a|--a3)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a3="$2"; shift 2 ;;
    -b|--a2)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a2="$2"; shift 2 ;;
    -c|--a1)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a1="$2"; shift 2 ;;
    -d|--a0)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a0="$2"; shift 2 ;;
    -i|--input)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      csv="$2"; shift 2 ;;
    -o|--output)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      output="$2"; shift 2 ;;
    --format)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      format="$2"; shift 2 ;;
    --margin)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      margin_factor="$2"; shift 2 ;;
    --point-size)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      point_size="$2"; shift 2 ;;
    --line-width)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      line_width="$2"; shift 2 ;;
    --curve-width)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      curve_width="$2"; shift 2 ;;
    --traj-color)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      traj_color="$2"; shift 2 ;;
    --curve-color)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      curve_color="$2"; shift 2 ;;
    --xrange)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      xrange_override="$2"; shift 2 ;;
    --title)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      custom_title="$2"; shift 2 ;;
    --show)
      show_flag=1; shift ;;
    --help)
      usage; exit 0 ;;
    *)
      echo "Unknown option: $1" >&2
      usage
      exit 1 ;;
  esac
done

if [ -z "$a3" ] || [ -z "$a2" ] || [ -z "$a1" ] || [ -z "$a0" ] || [ -z "$csv" ] || [ -z "$output" ]; then
  echo "Error: missing required arguments" >&2
  usage
  exit 1
fi

if [ ! -f "$csv" ]; then
  echo "Error: CSV file '$csv' not found" >&2
  exit 1
fi

if [ "$format" != "png" ] && [ "$format" != "svg" ]; then
  echo "Error: unsupported format '$format'" >&2
  exit 1
fi

output_dir=$(dirname "$output")
if [ ! -d "$output_dir" ]; then
  echo "Error: output directory '$output_dir' does not exist" >&2
  exit 1
fi

line_count=$(wc -l < "$csv")
if [ "$line_count" -lt 2 ]; then
  echo "Error: CSV '$csv' has no data rows" >&2
  exit 1
fi

if ! x_col=$(awk -F',' 'NR==1 { for (i=1;i<=NF;i++) if ($i=="x1") { print i; exit } exit 1 }' "$csv"); then
  echo "Error: column 'x1' not found in '$csv'" >&2
  exit 1
fi

if ! awk -F',' -v idx="$x_col" 'NR==2{ if($idx=="") exit 1 }' "$csv" >/dev/null 2>&1; then
  echo "Error: column 'x1' appears to be empty in '$csv'" >&2
  exit 1
fi

read data_xmin data_xmax <<EOF_INNER
$(awk -F',' -v idx="$x_col" 'NR==2{min=$idx; max=$idx} NR>1 { if($idx<min) min=$idx; if($idx>max) max=$idx } END { if(NR<=1){ print "nan nan"; exit 1 } else { printf "%s %s", min, max } }' "$csv")
EOF_INNER

if [ -z "$data_xmin" ] || [ "$data_xmin" = "nan" ]; then
  echo "Error: unable to determine x range from '$csv'" >&2
  exit 1
fi

start_x=$(awk -F',' -v idx="$x_col" 'NR==2 { print $idx; exit }' "$csv")
end_x=$(awk -F',' -v idx="$x_col" 'END { if(NR>1) print $idx }' "$csv")

roots=$(awk -v a3="$a3" -v a2="$a2" -v a1="$a1" 'BEGIN {
  eps = 1e-12
  A = 3.0 * a3
  B = 2.0 * a2
  C = a1
  if (A > -eps && A < eps) {
    if (B > eps || B < -eps) {
      root = -C / B
      printf "%.12g\n", root
    }
    exit 0
  }
  disc = B*B - 4.0*A*C
  if (disc > eps) {
    sqrt_disc = sqrt(disc)
    r1 = (-B - sqrt_disc) / (2.0*A)
    r2 = (-B + sqrt_disc) / (2.0*A)
    printf "%.12g %.12g\n", r1, r2
  } else if (disc >= -eps && disc <= eps) {
    root = -B / (2.0*A)
    printf "%.12g\n", root
  }
}')

range_min=""
range_max=""

if [ -n "$xrange_override" ]; then
  range_min=${xrange_override%%:*}
  range_max=${xrange_override##*:}
else
  if [ -n "$roots" ]; then
    set -- $roots
    if [ $# -eq 2 ]; then
      rmin=$(printf "%s\n%s\n" "$1" "$2" | sort -g | head -n1)
      rmax=$(printf "%s\n%s\n" "$1" "$2" | sort -g | tail -n1)
      span=$(awk -v a="$rmin" -v b="$rmax" 'BEGIN{ print (b-a) }')
      if awk -v s="$span" 'BEGIN{ exit !(s>0) }'; then
        margin=$(awk -v s="$span" -v m="$margin_factor" 'BEGIN{ print m*s }')
        range_min=$(awk -v r="$rmin" -v margin="$margin" 'BEGIN{ print r - margin }')
        range_max=$(awk -v r="$rmax" -v margin="$margin" 'BEGIN{ print r + margin }')
      fi
    elif [ $# -eq 1 ]; then
      root="$1"
      width=$(awk -v min="$data_xmin" -v max="$data_xmax" 'BEGIN{ print (max-min) }')
      if awk -v w="$width" 'BEGIN{ exit !(w>0) }'; then
        margin=$(awk -v w="$width" -v m="$margin_factor" 'BEGIN{ print m*w }')
      else
        margin=$(awk -v val="$root" 'BEGIN{ v = (val<0?-val:val); if (v<1) v=1; print 2*v }')
      fi
      range_min=$(awk -v r="$root" -v margin="$margin" 'BEGIN{ print r - margin }')
      range_max=$(awk -v r="$root" -v margin="$margin" 'BEGIN{ print r + margin }')
    fi
  fi

  if [ -z "$range_min" ] || [ -z "$range_max" ]; then
    width=$(awk -v min="$data_xmin" -v max="$data_xmax" 'BEGIN{ print (max-min) }')
    if ! awk -v w="$width" 'BEGIN{ exit (w>0) }'; then
      width=$(awk -v x="$data_xmin" 'BEGIN{ v=(x<0?-x:x); if (v<1) v=1; print 2*v }')
    fi
    margin=$(awk -v w="$width" -v m="$margin_factor" 'BEGIN{ print m*w }')
    center=$(awk -v min="$data_xmin" -v max="$data_xmax" 'BEGIN{ print (min+max)/2.0 }')
    range_min=$(awk -v c="$center" -v margin="$margin" 'BEGIN{ print c - margin }')
    range_max=$(awk -v c="$center" -v margin="$margin" 'BEGIN{ print c + margin }')
  fi
fi

marker_size=$(awk -v ps="$point_size" 'BEGIN{ print ps*1.2 }')

if [ -z "$custom_title" ]; then
  custom_title="Gradient descent on cubic: a3=$a3, a2=$a2, a1=$a1, a0=$a0"
fi

case "$format" in
  png)
    term_cmd="set terminal pngcairo size 900,600"
    ;;
  svg)
    term_cmd="set terminal svg size 900,600 fname 'Arial' fsize 12"
    ;;
  *)
    echo "Error: unsupported format '$format'" >&2
    exit 1
    ;;
esac

script_file=$(mktemp /tmp/plot_gd1d.XXXXXX.gp)
trap 'rm -f "$script_file"' EXIT

cat <<GNUPLOT > "$script_file"
$term_cmd
set output "$output"
set datafile separator ','
set title "$custom_title"
set xlabel "x"
set ylabel "f(x)"
set key top left
set grid
set xrange [$range_min:$range_max]

f(x) = $a3*x**3 + $a2*x**2 + $a1*x + $a0
xcol = $x_col
start_x = $start_x
end_x = $end_x
start_y = f(start_x)
end_y = f(end_x)

plot f(x) with lines lw $curve_width lc rgb "$curve_color" title "Objective", \
     "$csv" using (column(xcol)):(f(column(xcol))) with linespoints lw $line_width lc rgb "$traj_color" pt 7 ps $point_size title "Trajectory", \
     '-' with points pt 7 ps $marker_size lc rgb "$traj_color" title "Start", \
     '-' with points pt 7 ps $marker_size lc rgb "$curve_color" title "End"
start_x start_y
e
end_x end_y
e
GNUPLOT

if ! gnuplot "$script_file"; then
  echo "Error: gnuplot failed" >&2
  exit 1
fi

if [ "$show_flag" -eq 1 ]; then
  if command -v xdg-open >/dev/null 2>&1; then
    xdg-open "$output" >/dev/null 2>&1 || true
  else
    echo "Warning: xdg-open not available" >&2
  fi
fi

exit 0
