#!/bin/bash

# Configuration
PACKAGE_NAME="tde-win-deco-q4win10"
PACKAGE_VERSION="1.0"
DEB_VERSION="1.0"
ARCH=$(dpkg --print-architecture)
MAINTAINER="seb3773 <seb3773@github.com>"
DESCRIPTION="Q4WIN10 Window Decoration for Trinity Desktop"
BUILD_DIR="package_build"

# Detect Trinity version from parent directory or system
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# Try to extract version from path if possible, else default
TDE_VERSION=$(echo "$SCRIPT_DIR" | grep -oP 'trinity-\K[0-9]+\.[0-9]+\.[0-9]+' || echo "14.1.1")

# Package name format
DEB_NAME="${PACKAGE_NAME}_${PACKAGE_VERSION}_tde${TDE_VERSION}_${ARCH}.deb"

echo "Creating .deb package for $PACKAGE_NAME..."
echo "  Trinity version: $TDE_VERSION"
echo "  Architecture: $ARCH"

# Cleanup and setup
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR/opt/trinity/lib/trinity"
mkdir -p "$BUILD_DIR/opt/trinity/share/apps/twin"
mkdir -p "$BUILD_DIR/DEBIAN"

# Copy files
echo "Copying binaries and assets..."

# Main decoration
if [ -f "$SCRIPT_DIR/twin3_q4win10.so" ]; then
    cp "$SCRIPT_DIR/twin3_q4win10.so" "$BUILD_DIR/opt/trinity/lib/trinity/"
else
    echo "Error: twin3_q4win10.so not found. Please run 'make' first."
    exit 1
fi

# Config plugin
if [ -f "$SCRIPT_DIR/config/twin_q4win10_config.so" ]; then
    cp "$SCRIPT_DIR/config/twin_q4win10_config.so" "$BUILD_DIR/opt/trinity/lib/trinity/"
else
    echo "Warning: config/twin_q4win10_config.so not found. Config panel will be missing."
fi

# Desktop file
if [ -f "$SCRIPT_DIR/q4win10.desktop" ]; then
    cp "$SCRIPT_DIR/q4win10.desktop" "$BUILD_DIR/opt/trinity/share/apps/twin/"
fi

# Apply sstrip/strip
echo "Stripping binaries..."
for f in "$BUILD_DIR/opt/trinity/lib/trinity"/*.so; do
    if [ -f "$f" ]; then
        if command -v sstrip >/dev/null 2>&1; then
            sstrip "$f" 2>/dev/null || true
        else
            strip --strip-all "$f"
        fi
    fi
done

# Create control file
echo "Creating control file..."
cat <<EOF > "$BUILD_DIR/DEBIAN/control"
Package: $PACKAGE_NAME
Version: $DEB_VERSION
Section: x11
Priority: optional
Architecture: $ARCH
Depends: tqt3-mt, ltdecorations-trinity, tde-core-trinity, tdeui-trinity
Maintainer: $MAINTAINER
Description: $DESCRIPTION
 Windows 10 style decoration for Trinity.
 Highly optimized build.
 Author: seb3773 (https://github.com/seb3773)
EOF

# Build package
echo "Building package..."
dpkg-deb --build "$BUILD_DIR" "$DEB_NAME"

echo "Success! Package created: $DEB_NAME"
ls -lh "$DEB_NAME"
