#!/bin/bash
# BaranRamo macOS builder — universal (arm64 + x86_64) .app + zip.
# (c) 2026 Baran Avisat - All Rights Reserved.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
OUT="$HERE/dist"
APP="$OUT/BaranRamo.app"
SDKROOT="$(xcrun --show-sdk-path)"

mkdir -p "$OUT"

echo "== compile (arm64) =="
clang -fobjc-arc -O -target arm64-apple-macos12.0 \
    -framework Cocoa -framework WebKit "$HERE/main.m" -o "$HERE/bin-arm64"

echo "== compile (x86_64) =="
clang -fobjc-arc -O -target x86_64-apple-macos12.0 \
    -framework Cocoa -framework WebKit "$HERE/main.m" -o "$HERE/bin-x86_64"

echo "== universal binary =="
lipo -create "$HERE/bin-arm64" "$HERE/bin-x86_64" -o "$HERE/BaranRamo-bin"
file "$HERE/BaranRamo-bin"

echo "== bundle =="
rm -rf "$APP"
mkdir -p "$APP/Contents/MacOS" "$APP/Contents/Resources"
cp "$HERE/BaranRamo-bin" "$APP/Contents/MacOS/BaranRamo"
cp "$HERE/../game/BaranRamo.html" "$APP/Contents/Resources/BaranRamo.html"
cp "$HERE/../build/icons/AppIcon.icns" "$APP/Contents/Resources/AppIcon.icns"
cat > "$APP/Contents/Info.plist" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleName</key><string>BaranRamo</string>
	<key>CFBundleDisplayName</key><string>BaranRamo</string>
	<key>CFBundleIdentifier</key><string>com.baran.ramo</string>
	<key>CFBundleExecutable</key><string>BaranRamo</string>
	<key>CFBundleIconFile</key><string>AppIcon</string>
	<key>CFBundlePackageType</key><string>APPL</string>
	<key>CFBundleShortVersionString</key><string>1.0</string>
	<key>CFBundleVersion</key><string>1</string>
	<key>LSMinimumSystemVersion</key><string>12.0</string>
	<key>NSHighResolutionCapable</key><true/>
	<key>NSHumanReadableCopyright</key><string>© 2026 Baran Avisat — All Rights Reserved</string>
</dict>
</plist>
EOF
printf 'APPL????' > "$APP/Contents/PkgInfo"

echo "== codesign (ad-hoc) =="
codesign --force --deep --sign - "$APP"
codesign -dv "$APP" 2>&1 | grep -E "Identifier|Format" || true

echo "== zip =="
rm -f "$OUT/BaranRamo-macOS.zip"
ditto -c -k --sequesterRsrc --keepParent "$APP" "$OUT/BaranRamo-macOS.zip"
echo "DONE"
ls -la "$OUT"
