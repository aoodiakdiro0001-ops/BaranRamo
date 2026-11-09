#!/bin/bash
# BaranRamo Windows builder — mingw-w64 cross-compile (universal x86_64 PE).
# Produces a single offline .exe with the game embedded.
# (c) 2026 Baran Avisat - All Rights Reserved.
set -euo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
CC=x86_64-w64-mingw32-gcc
WINDRES=x86_64-w64-mingw32-windres
OUT="$HERE/dist"
mkdir -p "$OUT" "$HERE/work"

command -v "$CC" >/dev/null || { echo "mingw-w64 not installed (brew install mingw-w64)"; exit 1; }

if [ ! -f "$HERE/game_html.h" ]; then
    echo "== generate game_html.h =="
    python3 - "$HERE/../game/BaranRamo.html" "$HERE/game_html.h" <<'PYEOF'
import sys
data = open(sys.argv[1], "rb").read()
with open(sys.argv[2], "w", encoding="ascii") as f:
    f.write("/* Auto-generated: BaranRamo game (single-file HTML5). (c) 2026 Baran Avisat - All Rights Reserved. */\n")
    f.write("static const unsigned char game_html[] = {\n")
    for i in range(0, len(data), 20):
        f.write("  " + ",".join("0x%02x" % b for b in data[i:i+20]) + ",\n")
    f.write("};\n")
    f.write("static const unsigned int game_html_len = %d;\n" % len(data))
PYEOF
fi

echo "== windres (icon) =="
"$WINDRES" "$HERE/icon.rc" -O coff -o "$HERE/work/icon.o"

echo "== compile =="
"$CC" -O2 -s -mwindows -municode \
    -I "$HERE" \
    "$HERE/main.c" "$HERE/work/icon.o" \
    -lshell32 -lole32 -luuid \
    -o "$OUT/BaranRamo.exe"

echo "== verify =="
file "$OUT/BaranRamo.exe"
x86_64-w64-mingw32-objdump -p "$OUT/BaranRamo.exe" | grep -E "Subsystem|MajorOS" | head -2
echo "DONE"
ls -la "$OUT"
