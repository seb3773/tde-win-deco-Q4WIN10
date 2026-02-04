#!/bin/bash

# Configuration
PACKAGE_NAME="tdedecoration-twin-q4win10"
VERSION="2.0"
ARCH="amd64"
MAINTAINER="seb3773 <seb3773@github.com>"
DESCRIPTION="Q4WIN10 Window Decoration for Trinity Desktop Environment"
BUILD_DIR="package_build"
DEB_NAME="${PACKAGE_NAME}_${VERSION}_${ARCH}.deb"

echo "Creating .deb package for $PACKAGE_NAME..."

# Cleanup previous build
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR/opt/trinity/lib/trinity
mkdir -p $BUILD_DIR/opt/trinity/share/apps/twin
mkdir -p $BUILD_DIR/DEBIAN

# Build and Install (to temporary dir)
# Note: We assume the shared object is already built and stripped in ../build/
# But to be safe and use exact paths, we'll manually copy the artifacts
# since 'make install' hardcodes destination to /opt/trinity.

# Or better, we use the artifacts we just built in the build tree
BUILD_TREE_DIR="../../../build/twin/clients/q4win10"

echo "Copying files..."
if [ -f "$BUILD_TREE_DIR/twin3_q4win10.so" ]; then
    cp "$BUILD_TREE_DIR/twin3_q4win10.so" "$BUILD_DIR/opt/trinity/lib/trinity/"
    chmod 755 "$BUILD_DIR/opt/trinity/lib/trinity/twin3_q4win10.so"
else
    echo "Error: twin3_q4win10.so not found in build tree!"
    exit 1
fi

# Explicitly run sstrip to ensure maximum size reduction
echo "Applying sstrip..."
sstrip "$BUILD_DIR/opt/trinity/lib/trinity/twin3_q4win10.so"

echo "Copying config plugin..."
# Check if config plugin exists
if [ -f "$BUILD_TREE_DIR/config/twin_q4win10_config.so" ]; then
    cp "$BUILD_TREE_DIR/config/twin_q4win10_config.so" "$BUILD_DIR/opt/trinity/lib/trinity/"
    sstrip "$BUILD_DIR/opt/trinity/lib/trinity/twin_q4win10_config.so"
else
    echo "Warning: Config plugin not found!"
fi

# Copy desktop file
cp q4win10.desktop "$BUILD_DIR/opt/trinity/share/apps/twin/"

# Create control file
echo "Creating control file..."
cat <<EOF > $BUILD_DIR/DEBIAN/control
Package: $PACKAGE_NAME
Version: $VERSION
Section: x11
Priority: optional
Architecture: $ARCH
Depends: tdebase-trinity
Maintainer: $MAINTAINER
Description: $DESCRIPTION
 A flat, modern Windows 10 inspired window decoration for TDE.
 Optimized for speed and minimal size.
EOF

# Build package
echo "Building package..."
dpkg-deb --build $BUILD_DIR $DEB_NAME

echo "Success! Package created: $DEB_NAME"
ls -lh $DEB_NAME
