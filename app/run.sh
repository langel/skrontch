 #!/usr/bin/env bash
 set -euo pipefail
 
 EXE="./build/skrontch"
 
 if [ ! -f "$EXE" ]; then
     echo "Build first with ./build.sh"
     exit 1
 fi
 
 "$EXE"
