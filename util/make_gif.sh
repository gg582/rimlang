#!/usr/bin/env bash
# ==============================================================================
# RimLang: Terminal Session GIF Recorder
# Simulates natural human typing into the RimLang REPL and exports rimlang.gif
# ==============================================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${ROOT_DIR}"

echo "[1/3] Building RimLang..."
make -s rimlang

echo "[2/3] Recording natural interactive typing session..."
python3 util/record_session.py

echo "[3/3] Optimizing GIF..."
if command -v gifsicle >/dev/null 2>&1; then
    gifsicle -O3 --colors 128 rimlang.gif -o rimlang.gif
fi

echo "Done! Generated ${ROOT_DIR}/rimlang.gif"
