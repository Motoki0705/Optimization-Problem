#!/usr/bin/env bash
set -euo pipefail

# Directory (examples/)
SCRIPT_DIR="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTDIR="${OUTDIR:-"$SCRIPT_DIR/outputs/cubic"}"
mkdir -p "$OUTDIR"

# Function coefficients (default: x^3 - 4x^2 + 5x - 2)
A3="${A3:-1}"
A2="${A2:--4}"
A1="${A1:-5}"
A0="${A0:--2}"

# Sweep settings (defaults follow the problem statement)
ALPHAS="${ALPHAS:-"0.01 0.1 0.8"}"
INITS="${INITS:-"0.5 1.5 2.0 3.5 6.0"}"
EPS="${EPS:-0.01}"
MAXITERS="${MAXITERS:-100}"

# Build gd1d if needed
if [[ ! -x "$SCRIPT_DIR/gd1d.o" ]]; then
  echo "[build] gcc gd1d.c -> gd1d"
  gcc -std=c11 -O2 -Wall -Wextra "$SCRIPT_DIR/gd1d.c" -o "$SCRIPT_DIR/gd1d.o" -lm
fi

# Run sweeps → CSVs, then per-CSV plot → PNG
for a in $ALPHAS; do
  for x0 in $INITS; do
    tag="a${a}_x${x0}"
    tag_clean="$(echo "$tag" | tr '.' 'p' | tr '-' 'm')"
    csv="$OUTDIR/cubic_${tag_clean}.csv"
    png="$OUTDIR/plot_${tag_clean}.png"

    echo "[run] alpha=$a, x0=$x0 -> $(basename "$csv")"
    "$SCRIPT_DIR/gd1d.o" \
      --a3 "$A3" --a2 "$A2" --a1 "$A1" --a0 "$A0" \
      --alpha "$a" --eps "$EPS" --max-iters "$MAXITERS" \
      --x0 "$x0" --csv "$csv"

    echo "[plot] $(basename "$png")"
    gnuplot -e "CSV='$csv'; PNG='$png'; A3=$A3; A2=$A2; A1=$A1; A0=$A0; TITLE='GD (alpha=$a, x0=$x0)';" \
            "$SCRIPT_DIR/plot_single.gp"
  done
done

echo "[done] CSV/PNG created under: $OUTDIR"

