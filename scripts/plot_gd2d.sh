#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<USAGE
Usage: $(basename "$0") -A <a11> -B <a22> -C <a12> -p <b1> -q <b2> -r <c0> -i <csv> -o <output>

Options:
  -A, --a11 VALUE       Quadratic coefficient for x1^2 term (required)
  -B, --a22 VALUE       Quadratic coefficient for x2^2 term (required)
  -C, --a12 VALUE       Mixed coefficient for x1*x2 term (required)
  -p, --b1 VALUE        Linear coefficient for x1 term (required)
  -q, --b2 VALUE        Linear coefficient for x2 term (required)
  -r, --c0 VALUE        Constant term (required)
  -i, --input FILE      Gradient descent CSV trace (required)
  -o, --output FILE     Output image path (required)
      --format fmt      Output format: png (default) or svg
      --margin FACTOR   Fractional padding around trajectory bounds (default 0.25)
      --levels N        Number of contour levels (default 15)
      --point-size VAL  Point size for trajectory markers (default 1.0)
      --line-width VAL  Line width for trajectory path (default 1.7)
      --traj-color HEX  Colour for trajectory (default #d62728)
      --contour-color HEX Colour for contours (default #4c72b0)
      --xrange MIN:MAX  Override x range
      --yrange MIN:MAX  Override y range
      --title TEXT      Custom title
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

format="png"
margin_factor="0.25"
levels="15"
point_size="1.0"
line_width="1.7"
traj_color="#d62728"
contour_color="#4c72b0"
xrange_override=""
yrange_override=""
custom_title=""
show_flag=0

a11=""
a22=""
a12=""
b1=""
b2=""
c0=""
csv=""
output=""

while [ $# -gt 0 ]; do
  case "$1" in
    -A|--a11)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a11="$2"; shift 2 ;;
    -B|--a22)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a22="$2"; shift 2 ;;
    -C|--a12)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      a12="$2"; shift 2 ;;
    -p|--b1)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      b1="$2"; shift 2 ;;
    -q|--b2)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      b2="$2"; shift 2 ;;
    -r|--c0)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      c0="$2"; shift 2 ;;
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
    --levels)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      levels="$2"; shift 2 ;;
    --point-size)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      point_size="$2"; shift 2 ;;
    --line-width)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      line_width="$2"; shift 2 ;;
    --traj-color)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      traj_color="$2"; shift 2 ;;
    --contour-color)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      contour_color="$2"; shift 2 ;;
    --xrange)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      xrange_override="$2"; shift 2 ;;
    --yrange)
      [ $# -ge 2 ] || { echo "Error: option $1 requires an argument" >&2; exit 1; }
      yrange_override="$2"; shift 2 ;;
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

if [ -z "$a11" ] || [ -z "$a22" ] || [ -z "$a12" ] || [ -z "$b1" ] || [ -z "$b2" ] || [ -z "$c0" ] || [ -z "$csv" ] || [ -z "$output" ]; then
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

if ! y_col=$(awk -F',' 'NR==1 { for (i=1;i<=NF;i++) if ($i=="x2") { print i; exit } exit 1 }' "$csv"); then
  echo "Error: column 'x2' not found in '$csv'" >&2
  exit 1
fi

read x_min x_max <<EOF_X
$(awk -F',' -v idx="$x_col" 'NR==2{min=$idx; max=$idx} NR>1 { if($idx<min) min=$idx; if($idx>max) max=$idx } END { if(NR<=1){ print "nan nan"; exit 1 } else { printf "%s %s", min, max } }' "$csv")
EOF_X

read y_min y_max <<EOF_Y
$(awk -F',' -v idx="$y_col" 'NR==2{min=$idx; max=$idx} NR>1 { if($idx<min) min=$idx; if($idx>max) max=$idx } END { if(NR<=1){ print "nan nan"; exit 1 } else { printf "%s %s", min, max } }' "$csv")
EOF_Y

if [ -z "$x_min" ] || [ "$x_min" = "nan" ] || [ -z "$y_min" ] || [ "$y_min" = "nan" ]; then
  echo "Error: unable to determine bounds from '$csv'" >&2
  exit 1
fi

start_point=$(awk -F',' -v xi="$x_col" -v yi="$y_col" 'NR==2 { printf "%s %s", $xi, $yi; exit }' "$csv")
end_point=$(awk -F',' -v xi="$x_col" -v yi="$y_col" 'END { if(NR>1) printf "%s %s", $xi, $yi }' "$csv")
start_x=$(echo "$start_point" | awk '{print $1}')
start_y=$(echo "$start_point" | awk '{print $2}')
end_x=$(echo "$end_point" | awk '{print $1}')
end_y=$(echo "$end_point" | awk '{print $2}')

compute_range() {
  local min_val="$1" max_val="$2" override="$3"
  if [ -n "$override" ]; then
    echo "${override%%:*} ${override##*:}"
    return
  fi
  local width
  width=$(awk -v min="$min_val" -v max="$max_val" 'BEGIN{ print (max-min) }')
  if ! awk -v w="$width" 'BEGIN{ exit (w>0) }'; then
    width=$(awk -v m="$min_val" -v M="$max_val" 'BEGIN{ c=(m+M)/2.0; v=(c<0?-c:c); if (v<1) v=1; print 2*v }')
  fi
  local margin
  margin=$(awk -v w="$width" -v m="$margin_factor" 'BEGIN{ print m*w }')
  local center
  center=$(awk -v min="$min_val" -v max="$max_val" 'BEGIN{ print (min+max)/2.0 }')
  local rmin rmax
  rmin=$(awk -v c="$center" -v margin="$margin" 'BEGIN{ print c - margin }')
  rmax=$(awk -v c="$center" -v margin="$margin" 'BEGIN{ print c + margin }')
  echo "$rmin $rmax"
}

read range_xmin range_xmax <<<"$(compute_range "$x_min" "$x_max" "$xrange_override")"
read range_ymin range_ymax <<<"$(compute_range "$y_min" "$y_max" "$yrange_override")"

if [ -z "$custom_title" ]; then
  custom_title="Gradient descent on quadratic form"
fi

case "$format" in
  png)
    term_cmd="set terminal pngcairo size 900,700"
    ;;
  svg)
    term_cmd="set terminal svg size 900,700 fname 'Arial' fsize 12"
    ;;
  *)
    echo "Error: unsupported format '$format'" >&2
    exit 1
    ;;
esac

contour_file=$(mktemp /tmp/gd2d_contours.XXXXXX.dat)
script_file=$(mktemp /tmp/plot_gd2d.XXXXXX.gp)
trap 'rm -f "$contour_file" "$script_file"' EXIT

cat <<GNUPLOT > "$script_file"
$term_cmd
set output "$output"
set datafile separator ','
set title "$custom_title"
set xlabel "x_1"
set ylabel "x_2"
set key outside right
set grid
set xrange [$range_xmin:$range_xmax]
set yrange [$range_ymin:$range_ymax]
set view map
unset surface
set contour base
set cntrparam levels $levels
set isosamples 60,60

f(x,y) = $a11*x**2 + $a22*y**2 + $a12*x*y + $b1*x + $b2*y + $c0

set table "$contour_file"
splot f(x,y)
unset table

plot \
  "$contour_file" using 1:2 with lines lc rgb "$contour_color" lw 1 title "Contours", \
  "$csv" using $x_col:$y_col with lines lw $line_width lc rgb "$traj_color" title "Trajectory", \
  "$csv" using $x_col:$y_col with points pt 7 ps $point_size lc rgb "$traj_color" notitle, \
  '-' using 1:2 with points pt 7 ps $(awk -v ps="$point_size" 'BEGIN{ print ps*1.3 }') lc rgb "$traj_color" title "Start", \
  '-' using 1:2 with points pt 7 ps $(awk -v ps="$point_size" 'BEGIN{ print ps*1.3 }') lc rgb "$contour_color" title "End"
$start_x $start_y
e
$end_x $end_y
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
