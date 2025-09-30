#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
EXAMPLE="2d"

while [[ $# -gt 0 ]]; do
    case "$1" in
        --example)
            EXAMPLE="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [--example 1d|2d]"
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" >/dev/null

case "$EXAMPLE" in
    1d)
        cmake --build "$BUILD_DIR" --target gd1d >/dev/null
        OUTPUT_DIR="$ROOT_DIR/topics/gradient_descent/examples/outputs"
        mkdir -p "$OUTPUT_DIR"
        CSV_PATH="$OUTPUT_DIR/gd1d.csv"
        "$BUILD_DIR/topics/gradient_descent/gd1d" \
            --a3 1 --a2 -6 --a1 11 --a0 -6 \
            --alpha 0.0005 --eps 1e-6 --max-iters 2000 \
            --x0 2.0 --csv "$CSV_PATH"
        echo "Generated 1D gradient descent trace at $CSV_PATH"
        ;;
    2d)
        cmake --build "$BUILD_DIR" --target gd2d >/dev/null
        OUTPUT_DIR="$ROOT_DIR/topics/gradient_descent/examples/outputs"
        mkdir -p "$OUTPUT_DIR"
        CSV_PATH="$OUTPUT_DIR/gd2d.csv"
        "$BUILD_DIR/topics/gradient_descent/gd2d" \
            --a11 1 --a22 1 --a12 0 --b1 -2 --b2 4 --c0 5 \
            --alpha 0.1 --eps 1e-6 --max-iters 300 \
            --x1 0.0 --x2 0.0 --csv "$CSV_PATH"
        echo "Generated 2D gradient descent trace at $CSV_PATH"
        ;;
    *)
        echo "Unsupported example: $EXAMPLE" >&2
        exit 1
        ;;

esac
