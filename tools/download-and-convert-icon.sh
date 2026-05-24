#!/bin/bash

# Pebble Icon Downloader & Converter
#
# Downloads an SVG icon from a URL and converts it to PNG for Pebble apps.
#
# Usage:
#   ./download-and-convert-icon.sh <url> [name] [size]
#
# Examples:
#   ./download-and-convert-icon.sh https://example.org/icons/train-medium.svg
#   ./download-and-convert-icon.sh https://example.org/icons/train-medium.svg train-icon
#   ./download-and-convert-icon.sh https://example.org/icons/train-medium.svg train-icon 24

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
RESOURCES_DIR="$PROJECT_ROOT/resources/images"

# Show usage
if [ $# -eq 0 ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    cat << 'EOF'
Pebble Icon Downloader & Converter

Downloads an SVG icon from a URL and converts it to PNG for Pebble apps.

Usage:
  ./download-and-convert-icon.sh <url> [name] [size]

Arguments:
  url     - URL to download the SVG from (required)
  name    - Base name for the output files (optional, defaults to URL filename)
  size    - Icon size in pixels (optional, defaults to 18)

Examples:
  ./download-and-convert-icon.sh https://example.org/icons/train-medium.svg
  ./download-and-convert-icon.sh https://example.org/icons/train-medium.svg train-icon
  ./download-and-convert-icon.sh https://example.org/icons/train-medium.svg train-icon 24

EOF
    exit 0
fi

URL="$1"
SIZE="${3:-18}"

# Determine output name
if [ -n "$2" ]; then
    NAME="$2"
else
    # Extract filename from URL, remove extension
    NAME=$(basename "$URL" .svg)
fi

SVG_PATH="$RESOURCES_DIR/$NAME.svg"
PNG_PATH="$RESOURCES_DIR/$NAME.png"

echo "Downloading icon from: $URL"
curl -L -o "$SVG_PATH" "$URL"
echo "✓ Saved SVG: $SVG_PATH"

echo ""
echo "Converting to PNG (${SIZE}x${SIZE})..."
node "$SCRIPT_DIR/convert-icon.js" "$SVG_PATH" "$PNG_PATH" "$SIZE"

echo ""
echo "Done! Add this to your appinfo.json resources:"
echo ""
cat << EOF
{
  "type": "bitmap",
  "name": "$(echo "$NAME" | tr '[:lower:]' '[:upper:]' | tr '-' '_')",
  "file": "images/$NAME.png"
}
EOF
