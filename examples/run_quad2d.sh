#!/usr/bin/env bash
set -euo pipefail

# このスクリプトのディレクトリ（= examples）
SCRIPT_DIR="$(cd -- "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
OUTDIR="${OUTDIR:-"$SCRIPT_DIR/outputs/quad2d"}"
mkdir -p "$OUTDIR"

# --- 関数係数（課題の既定値） ---
A11="${A11:-3}"
A22="${A22:-2}"
A12="${A12:--2}"
B1="${B1:--18}"
B2="${B2:--8}"
C0="${C0:-0}"

# --- 反復条件（デフォルトは無難な例） ---
ALPHAS="${ALPHAS:-"0.05 0.1 0.2"}"   # 学習率の集合
EPS="${EPS:-0.01}"                    # 収束判定（||grad||2）
MAXITERS="${MAXITERS:-200}"           # 最大反復回数

# 初期値の集合（スペース区切り、各要素は "x1,x2"）
INITS="${INITS:-"0,0  5,5  10,0  -2,8  6,1"}"

# --- ビルド ---
if [[ ! -x "$SCRIPT_DIR/gd2d.o" ]]; then
  echo "[build] gcc gd2d.c -> gd2d.o"
  gcc -std=c11 -O2 -Wall -Wextra "$SCRIPT_DIR/gd2d.c" -o "$SCRIPT_DIR/gd2d.o" -lm
fi

# --- 総当たりで実行（CSV 1本/組み合わせ） ---
for a in $ALPHAS; do
  for p in $INITS; do
    x1="${p%%,*}"
    x2="${p##*,}"
    tag="a${a}_x${x1}_${x2}"
    tag_clean="$(echo "$tag" | tr '.' 'p' | tr '-' 'm')"
    csv="$OUTDIR/quad2d_${tag_clean}.csv"

    echo "[run] alpha=$a, x=($x1,$x2) -> $(basename "$csv")"
    "$SCRIPT_DIR/gd2d.o" \
      --a11 "$A11" --a22 "$A22" --a12 "$A12" \
      --b1 "$B1" --b2 "$B2" --c0 "$C0" \
      --alpha "$a" --eps "$EPS" --max-iters "$MAXITERS" \
      --x1 "$x1" --x2 "$x2" --csv "$csv"
  done
done

echo "[done] CSV files at: $OUTDIR"

