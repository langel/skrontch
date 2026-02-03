#!/usr/bin/env bash
set -eu
if (set -o pipefail) 2>/dev/null; then
    set -o pipefail
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
APP_NAME="skrontch"
BUILD_DIR="$SCRIPT_DIR/build"
APP_OUTPUT_DIR="${APP_OUTPUT_DIR:-$BUILD_DIR}"
APP_DIR="$APP_OUTPUT_DIR/${APP_NAME}.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"

"$SCRIPT_DIR/build.sh"

ICON_SRC="$SCRIPT_DIR/assets/skrontch_icon_1024.png"
ICONSET_DIR="$BUILD_DIR/${APP_NAME}.iconset"
rm -rf "$APP_DIR" "$ICONSET_DIR"
mkdir -p "$APP_OUTPUT_DIR" "$MACOS_DIR" "$RESOURCES_DIR" "$ICONSET_DIR"

cp "$BUILD_DIR/skrontch" "$MACOS_DIR/$APP_NAME"
chmod +x "$MACOS_DIR/$APP_NAME"
rm -rf "$RESOURCES_DIR/assets"
cp -R "$SCRIPT_DIR/assets" "$RESOURCES_DIR/assets"

sips -z 16 16   "$ICON_SRC" --out "$ICONSET_DIR/icon_16x16.png" >/dev/null
sips -z 32 32   "$ICON_SRC" --out "$ICONSET_DIR/icon_16x16@2x.png" >/dev/null
sips -z 32 32   "$ICON_SRC" --out "$ICONSET_DIR/icon_32x32.png" >/dev/null
sips -z 64 64   "$ICON_SRC" --out "$ICONSET_DIR/icon_32x32@2x.png" >/dev/null
sips -z 128 128 "$ICON_SRC" --out "$ICONSET_DIR/icon_128x128.png" >/dev/null
sips -z 256 256 "$ICON_SRC" --out "$ICONSET_DIR/icon_128x128@2x.png" >/dev/null
sips -z 256 256 "$ICON_SRC" --out "$ICONSET_DIR/icon_256x256.png" >/dev/null
sips -z 512 512 "$ICON_SRC" --out "$ICONSET_DIR/icon_256x256@2x.png" >/dev/null
sips -z 512 512 "$ICON_SRC" --out "$ICONSET_DIR/icon_512x512.png" >/dev/null
sips -z 1024 1024 "$ICON_SRC" --out "$ICONSET_DIR/icon_512x512@2x.png" >/dev/null

ICNS_TEMP="$BUILD_DIR/${APP_NAME}.icns"
if ! iconutil -c icns "$ICONSET_DIR" -o "$ICNS_TEMP"; then
    echo "Failed to generate .icns with iconutil."
    echo "Check that iconutil is available and the iconset is valid."
    exit 1
fi
mv "$ICNS_TEMP" "$RESOURCES_DIR/${APP_NAME}.icns"
rm -rf "$ICONSET_DIR"
touch "$APP_DIR"

cat > "$CONTENTS_DIR/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleDisplayName</key>
    <string>${APP_NAME}</string>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME}</string>
    <key>CFBundleIdentifier</key>
<string>com.skrontch.app</string>
    <key>CFBundleIconFile</key>
    <string>${APP_NAME}</string>
    <key>CFBundleName</key>
    <string>${APP_NAME}</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>0.1.0</string>
    <key>CFBundleVersion</key>
    <string>0.1.0</string>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
EOF

echo "App bundle created at: $APP_DIR"
