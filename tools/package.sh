#!/usr/bin/env bash
# Build the two upload archives for Part 2:
#   dist/aggieforge-traffic-cloud-code.zip  - Canvas "All code and make file" (sources only)
#   dist/docs.zip                           - Doxygen HTML for people.tamu.edu (expands to docs/)
set -euo pipefail
cd "$(dirname "$0")/.."
mkdir -p dist
rm -f dist/aggieforge-traffic-cloud-code.zip dist/docs.zip

zip -q dist/aggieforge-traffic-cloud-code.zip Makefile config.txt README.md Doxyfile ./*.cpp ./*.h
echo "code zip:"; unzip -Z1 dist/aggieforge-traffic-cloud-code.zip | sed 's/^/  /'

if [ -d docs/html ]; then
    STAGE=$(mktemp -d)
    cp -R docs/html "$STAGE/docs"
    (cd "$STAGE" && zip -qr "$OLDPWD/dist/docs.zip" docs)
    rm -rf "$STAGE"
    echo "docs zip: $(unzip -l dist/docs.zip | tail -1 | awk '{print $2}') files"
else
    echo "docs/html missing - run 'make docs' first" >&2
fi
ls -la dist
