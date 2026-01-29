#!/usr/bin/env bash
set -eu
if (set -o pipefail) 2>/dev/null; then
    set -o pipefail
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"$SCRIPT_DIR/build.sh"
"$SCRIPT_DIR/run.sh"
