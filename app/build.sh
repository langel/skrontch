 #!/usr/bin/env bash
 set -euo pipefail
 
 SRC_DIR="src"
 BUILD_DIR="build"
 INCLUDE_DIR="include"
 
 mkdir -p "$BUILD_DIR"
 
CFLAGS="-std=c99 -Wall -Wextra -I$INCLUDE_DIR $(sdl2-config --cflags)"
LDFLAGS="$(sdl2-config --libs) -lSDL2_image"
 
 SOURCES=$(find "$SRC_DIR" -name "*.c")
 
 if [ -z "$SOURCES" ]; then
     echo "No C source files found under $SRC_DIR."
     exit 1
 fi
 
 gcc $CFLAGS $SOURCES -o "$BUILD_DIR/skrontch" $LDFLAGS
 
 echo "Build complete: $BUILD_DIR/skrontch"
