#!/bin/bash
# Render an SVG to PNG with headless Chrome (no rsvg/inkscape on this Mac).
# usage: tools/render-png.sh in.svg out.png WIDTH HEIGHT
set -e
IN="$1"; OUT="$2"; W="$3"; H="$4"; rm -f "$OUT"
CHROME="/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"
TMP=$(mktemp -d)
cat > "$TMP/wrap.html" <<HTML
<!doctype html><html><head><meta charset="utf-8"><style>html,body{margin:0;padding:0;background:#F5F2EC}img{display:block;width:${W}px;height:${H}px}</style></head>
<body><img src="file://$(cd "$(dirname "$IN")" && pwd)/$(basename "$IN")"></body></html>
HTML
"$CHROME" --headless --disable-gpu --no-sandbox --user-data-dir="$TMP/prof" --allow-file-access-from-files \
  --hide-scrollbars --force-device-scale-factor=1 --window-size="${W},${H}" \
  --screenshot="$OUT" "file://$TMP/wrap.html" >/dev/null 2>&1 &
PID=$!
for i in $(seq 1 60); do sleep 0.5; [ -s "$OUT" ] && break; done
sleep 1; kill $PID 2>/dev/null || true
rm -rf "$TMP"
ls -la "$OUT"
