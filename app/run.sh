#!/usr/bin/env bash
set -eu
if (set -o pipefail) 2>/dev/null; then
    set -o pipefail
fi

EXE="./build/skrontch"

if [ ! -f "$EXE" ]; then
    echo "Build first with ./build.sh"
    exit 1
fi

"$EXE"
