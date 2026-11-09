#!/bin/bash
# BaranRamo packager — assembles dist/ bundles for GitHub Release, Gumroad & friends.
# (c) 2026 Baran Avisat - All Rights Reserved.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
DIST="$HERE/dist"
GAME="$HERE/game/BaranRamo.html"
README="$HERE/delivery/README.md"

[ -f "$GAME" ] || { echo "missing game"; exit 1; }

rm -rf "$DIST"
mkdir -p "$DIST"

# ---------- 1. Android APKs ----------
[ -f "$HERE/android/dist/BaranRamo-Android-Compat.apk" ] || { echo "run android/build.sh first"; exit 1; }
cp "$HERE/android/dist/BaranRamo-Android-Compat.apk" "$DIST/"
cp "$HERE/android/dist/BaranRamo-Android-Modern.apk" "$DIST/"

# ---------- 2. Windows zip ----------
[ -f "$HERE/windows/dist/BaranRamo.exe" ] || { echo "run windows/build.sh first"; exit 1; }
W="$DIST/BaranRamo-Windows"
mkdir -p "$W"
cp "$HERE/windows/dist/BaranRamo.exe" "$W/"
cat > "$W/HOW-TO-RUN.txt" <<'EOF'
BaranRamo — ويندوز
© 2026 Baran Avisat — جميع الحقوق محفوظة

التشغيل:
1) اضغط نقرة مزدوجة على BaranRamo.exe — تُفتح اللعبة في المتصفح مباشرة (تعمل بدون إنترنت).
2) أول مرة قد يظهر تحذير SmartScreen من Windows: اضغط «مزيد من المعلومات» ثم «تشغيل على أي حال».
   (سبب التحذير: الملف غير موقَّع بشهادة مدفوعة — عادي تماماً للألعاب المستقلة).
3) حفظك في اللعبة يُخزَّن تلقائياً في: AppData\Local\BaranRamo

التواصل: Baranavisat@gmail.com · واتساب +90 531 326 37 22
EOF
(cd "$DIST" && zip -q -r -X "BaranRamo-Windows.zip" "BaranRamo-Windows")
rm -rf "$W"

# ---------- 3. macOS zip (built by macos/build.sh) ----------
[ -f "$HERE/macos/dist/BaranRamo-macOS.zip" ] || { echo "run macos/build.sh first"; exit 1; }
cp "$HERE/macos/dist/BaranRamo-macOS.zip" "$DIST/"

# ---------- 4. HTML5 zip ----------
H="$DIST/BaranRamo-HTML5"
mkdir -p "$H"
cp "$GAME" "$H/"
cp "$README" "$H/README.md"
(cd "$DIST" && zip -q -r -X "BaranRamo-HTML5.zip" "BaranRamo-HTML5")
rm -rf "$H"

# ---------- 5. Friends zip (games only — the file to send friends) ----------
# NOTE: ASCII paths inside zips on purpose (no mojibake on Windows). Arabic text lives INSIDE files.
F="$DIST/BaranRamo-Friends"
mkdir -p "$F/Android" "$F/Windows" "$F/macOS" "$F/iPhone"
cp "$DIST/BaranRamo-Android-Compat.apk" "$F/Android/"
cp "$DIST/BaranRamo-Android-Modern.apk" "$F/Android/"
cp "$HERE/windows/dist/BaranRamo.exe" "$F/Windows/"
cp "$GAME" "$F/BaranRamo.html"
cp -R "$HERE/macos/dist/BaranRamo.app" "$F/macOS/"
cp "$README" "$F/README.md"
cat > "$F/iPhone/INSTALL.txt" <<'EOF'
تثبيت BaranRamo على الآيفون:
1) افتح هذا الرابط في Safari:
   https://aoodiakdiro0001-ops.github.io/BaranRamo/
2) زر المشاركة (المربع + سهم) ← «إضافة إلى الشاشة الرئيسية» ← «إضافة».
3) تظهر أيقونة اللعبة كتطبيق كامل ويعمل بدون موقع داخله.
© 2026 Baran Avisat
EOF
(cd "$DIST" && zip -q -r -X "BaranRamo-Friends.zip" "BaranRamo-Friends")
rm -rf "$F"

# ---------- 6. Complete bundle (Gumroad) ----------
C="$DIST/BaranRamo-Complete"
mkdir -p "$C/Android" "$C/Windows" "$C/macOS" "$C/iOS" "$C/HTML5" "$C/docs"
cp "$DIST/BaranRamo-Android-Compat.apk" "$C/Android/"
cp "$DIST/BaranRamo-Android-Modern.apk" "$C/Android/"
cp "$HERE/windows/dist/BaranRamo.exe" "$C/Windows/"
cp "$GAME" "$C/HTML5/"
cp "$README" "$C/README.md"
cp "$DIST/BaranRamo-Windows.zip" "$C/Windows/"
cp "$DIST/BaranRamo-macOS.zip" "$C/macOS/"
cp "$HERE/docs/BaranRamo-GDD.md" "$C/docs/"
cp "$HERE/docs/BaranRamo-TechRoadmap.md" "$C/docs/"
cat > "$C/معلومات-الترخيص.txt" <<'EOF'
BaranRamo — الترخيص وحقوق الملكية
© 2026 Baran Avisat — جميع الحقوق محفوظة.

المؤلف والمصمم والمخرج: Baran Avisat
- يُمنح المشتري حق استخدام اللعبة للاستخدام الشخصي.
- لا يُسمح بإعادة النشر أو البيع أو توزيع اللعبة أو تعديلها ودَّعي حقوق ملكية جديدة.
- اللعبة مبنية بموارد مجانية 100% ولا تحتوي أي تتبّع أو إعلانات.

تواصل معي: Baranavisat@gmail.com · واتساب +90 531 326 37 22
GitHub: aoodiakdiro0001-ops
EOF
cat > "$C/iOS/تثبيت-الآيفون.txt" <<'EOF'
تثبيت BaranRamo على الآيفون / الآيباد:
1) افتح في Safari: https://aoodiakdiro0001-ops.github.io/BaranRamo/
2) زر المشاركة ← «إضافة إلى الشاشة الرئيسية» ← «إضافة».
3) يعمل كتطبيق كامل بدون إنترنت بعد أول فتح.
© 2026 Baran Avisat
EOF
(cd "$DIST" && zip -q -r -X "BaranRamo-Complete.zip" "BaranRamo-Complete")
rm -rf "$C"

echo "DONE"
ls -la "$DIST"/*.zip "$DIST"/*.apk
