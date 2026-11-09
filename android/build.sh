#!/bin/bash
# BaranRamo Android builder — manual pipeline (no Gradle).
# Builds two APKs covering ALL Android versions:
#   Compat : minSdk 21  -> Android 5.0+  (every device)
#   Modern : minSdk 26  -> Android 8.0+  (newer devices, same game)
# © 2026 Baran Avisat — All Rights Reserved.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SDK="${ANDROID_HOME:-/opt/homebrew/share/android-commandlinetools}"
BT="$SDK/build-tools/34.0.0"
AJAR="$SDK/platforms/android-34/android.jar"
KS="$HERE/keystore/baranramo.keystore"
PASS="$(head -1 "$HERE/keystore/credentials.txt")"
DIST="$HERE/dist"
WORK="$HERE/work"

[ -f "$AJAR" ] || { echo "missing android.jar at $AJAR"; exit 1; }
[ -f "$KS" ] || { echo "missing keystore"; exit 1; }

rm -rf "$WORK"
mkdir -p "$WORK/classes" "$WORK/dex" "$DIST"

echo "== 1/7 javac =="
SRCS=()
while IFS= read -r f; do SRCS+=("$f"); done < <(find "$HERE/src" -name '*.java')
javac --release 11 -classpath "$AJAR" -d "$WORK/classes" "${SRCS[@]}"

echo "== 2/7 d8 -> classes.dex =="
CLASSES=()
while IFS= read -r f; do CLASSES+=("$f"); done < <(find "$WORK/classes" -name '*.class')
"$BT/d8" --release --lib "$AJAR" --min-api 21 --output "$WORK/dex" "${CLASSES[@]}"

echo "== 3/7 aapt2 compile resources =="
"$BT/aapt2" compile --dir "$HERE/res" -o "$WORK/res.zip"

build_apk () {
  local MIN="$1" LABEL="$2" OUT="$3"
  echo "== build $LABEL (minSdk $MIN) =="
  local APK="$WORK/base-$LABEL.apk"
  rm -f "$APK" "$WORK/aligned-$LABEL.apk"
  "$BT/aapt2" link -I "$AJAR" \
      --manifest "$HERE/AndroidManifest.xml" \
      -o "$APK" \
      --min-sdk-version "$MIN" --target-sdk-version 34 \
      --version-code 1 --version-name 1.0 \
      --auto-add-overlay \
      -A "$HERE/assets" \
      "$WORK/res.zip"
  # inject dex
  ( cd "$WORK/dex" && zip -q -j "$APK" classes.dex )
  "$BT/zipalign" -f 4 "$APK" "$WORK/aligned-$LABEL.apk"
  rm -f "$OUT"
  "$BT/apksigner" sign --ks "$KS" \
      --ks-pass "pass:$PASS" --key-pass "pass:$PASS" \
      --out "$OUT" "$WORK/aligned-$LABEL.apk"
  "$BT/apksigner" verify --print-certs "$OUT" > "$WORK/verify-$LABEL.txt" 2>&1
  echo "   -> $OUT ($(du -h "$OUT" | cut -f1))"
}

echo "== 4-6/7 build + align + sign =="
build_apk 21 compat  "$DIST/BaranRamo-Android-Compat.apk"
build_apk 26 modern  "$DIST/BaranRamo-Android-Modern.apk"

echo "== 7/7 verify =="
for f in "$DIST"/*.apk; do
  "$BT/apksigner" verify "$f" >/dev/null 2>&1 && echo "   OK signed: $(basename "$f")" || { echo "   FAIL: $f"; exit 1; }
done
echo "DONE"
ls -la "$DIST"
