#!/usr/bin/env bash
set -eu
if (set -o pipefail) 2>/dev/null; then
	set -o pipefail
fi

BUILD_MODE="dev"
UNITY_MODE="1"
for arg in "$@"; do
	case "$arg" in
		dev)
			BUILD_MODE="dev"
			;;
		release)
			BUILD_MODE="release"
			;;
		unity)
			UNITY_MODE="1"
			;;
		nounity)
			UNITY_MODE="0"
			;;
		*)
			echo "Unknown argument: $arg"
			echo "Usage: ./build.sh [dev|release] [unity|nounity]"
			exit 1
			;;
	esac
done

SRC_DIR="src"
BUILD_DIR="build"
INCLUDE_DIR="include"
OUTPUT_DIR="$BUILD_DIR"
if [ "$BUILD_MODE" = "release" ]; then
	OUTPUT_DIR="$BUILD_DIR/release"
fi

mkdir -p "$OUTPUT_DIR"

SDL_CFLAGS="$(sdl2-config --cflags)"
SDL_LIBS="$(sdl2-config --libs)"

CFLAGS="-std=c99 -Wall -Wextra -I$INCLUDE_DIR -I$SRC_DIR $SDL_CFLAGS"
LDFLAGS="$SDL_LIBS"

if [ "$UNITY_MODE" = "1" ]; then
	SOURCES="$SRC_DIR/unity_build.c"
	if [ ! -f "$SOURCES" ]; then
		echo "Unity source file not found: $SOURCES"
		exit 1
	fi
	echo "[$BUILD_MODE] Unity build enabled."
else
	SOURCES=$(find "$SRC_DIR" -name "*.c" ! -name "unity_build.c")
fi

if [ -z "$SOURCES" ]; then
	echo "No C source files found under $SRC_DIR."
	exit 1
fi

gcc $CFLAGS $SOURCES -o "$OUTPUT_DIR/skrontch" $LDFLAGS

echo "Build complete [$BUILD_MODE]: $OUTPUT_DIR/skrontch"
