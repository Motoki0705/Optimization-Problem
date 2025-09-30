#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT_DIR/build}"
DEFAULT_INPUT="$ROOT_DIR/topics/simplex/examples/sample.lp"
INPUT_PATH="$DEFAULT_INPUT"

echo "$ROOT_DIR" >&2
while [[ $# -gt 0 ]]; do
    case "$1" in
        --input)
            INPUT_PATH="$2"
            shift 2
            ;;
        --help|-h)
            echo "Usage: $0 [--input path]"
            echo "Default input: $DEFAULT_INPUT"
            exit 0
            ;;
        *)
            echo "Unknown argument: $1" >&2
            exit 1
            ;;
    esac
done

if [[ ! -f "$INPUT_PATH" ]]; then
    echo "Input file not found: $INPUT_PATH" >&2
    exit 1
fi

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" >/dev/null
cmake --build "$BUILD_DIR" --target simplex_cli >/dev/null

"$BUILD_DIR/topics/simplex/simplex_cli" --input "$INPUT_PATH"
