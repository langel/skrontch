#!/usr/bin/env bash
set -eu
if (set -o pipefail) 2>/dev/null; then
    set -o pipefail
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

"$SCRIPT_DIR/build-mac-app.sh"
open "$SCRIPT_DIR/build/skrontch.app"
