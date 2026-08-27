#!/bin/bash
set -e

PACKAGE_NAME="tde-win-deco-q4win10"
INSTALL_FLAG=0
VERSION_ARG=""

for arg in "$@"; do
    if [ "$arg" = "--install" ] || [ "$arg" = "-i" ]; then
        INSTALL_FLAG=1
    elif [ -z "$VERSION_ARG" ] && [ "${arg#-}" = "$arg" ]; then
        VERSION_ARG="$arg"
    fi
done

BUILD_TIMESTAMP=$(date +%Y%m%d.%H%M%S)

if [ -n "$VERSION_ARG" ]; then
    PACKAGE_VERSION="$VERSION_ARG"
    DEB_VERSION="${PACKAGE_VERSION}-1"
else
    PACKAGE_VERSION="2.0.1"
    DEB_VERSION="${PACKAGE_VERSION}"
fi

ARCH=$(dpkg --print-architecture)
MAINTAINER="seb3773 <seb3773@github.com>"
DESCRIPTION="Q4WIN10 Window Decoration for Trinity Desktop Environment"
BUILD_DIR="package_build"
DEB_NAME="${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCH}.deb"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_ROOT="$SCRIPT_DIR/build"

echo "Updating version header ($PACKAGE_VERSION / Build $BUILD_TIMESTAMP)..."
cat <<EOF > "$SCRIPT_DIR/q4win10_version.h"
#ifndef Q4WIN10_VERSION_H
#define Q4WIN10_VERSION_H

#define Q4WIN10_VERSION "$PACKAGE_VERSION"
#define Q4WIN10_BUILD_TIMESTAMP "$BUILD_TIMESTAMP"
#define Q4WIN10_VERSION_STRING "Version $PACKAGE_VERSION (Build $BUILD_TIMESTAMP)"

#endif // Q4WIN10_VERSION_H
EOF

echo "Embedding images/logo..."
python3 "$SCRIPT_DIR/convert_images.py"

echo "=============================================="
echo "Creating universal .deb package: $PACKAGE_NAME"
echo "  Target: TDE R14.1.x (R14.1.0 - R14.1.6+)"
echo "  Version: $PACKAGE_VERSION"
echo "  Architecture: $ARCH"
echo "=============================================="

# Build binaries using CMake
echo "Building decoration with CMake and aggressive optimization..."
mkdir -p "$BUILD_ROOT"
cd "$BUILD_ROOT"
cmake "$SCRIPT_DIR" -DCMAKE_INSTALL_PREFIX=/opt/trinity -DQ4WIN10_AGGRESSIVE_FLAGS=ON
make -j"$(nproc)"

cd "$SCRIPT_DIR"

echo "Preparing package tree..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR/opt/trinity/lib/trinity"
mkdir -p "$BUILD_DIR/opt/trinity/share/apps/twin"
mkdir -p "$BUILD_DIR/DEBIAN"

# Install via CMake into package directory
DESTDIR="$SCRIPT_DIR/$BUILD_DIR" make -C "$BUILD_ROOT" install

# Ensure binaries are stripped aggressively
echo "Stripping binaries..."
for f in "$BUILD_DIR/opt/trinity/lib/trinity"/*.so; do
    if [ -f "$f" ]; then
        if command -v sstrip >/dev/null 2>&1; then
            sstrip "$f" 2>/dev/null || true
        else
            strip --strip-all "$f" 2>/dev/null || true
        fi
    fi
done

# Post-install script
cat <<EOF > "$BUILD_DIR/DEBIAN/postinst"
#!/bin/sh
set -e
if [ -x /opt/trinity/bin/tdebuildsycoca ]; then
    /opt/trinity/bin/tdebuildsycoca >/dev/null 2>&1 || true
fi
exit 0
EOF
chmod 755 "$BUILD_DIR/DEBIAN/postinst"

# Post-removal script
cat <<EOF > "$BUILD_DIR/DEBIAN/postrm"
#!/bin/sh
set -e
if [ -x /opt/trinity/bin/tdebuildsycoca ]; then
    /opt/trinity/bin/tdebuildsycoca >/dev/null 2>&1 || true
fi
exit 0
EOF
chmod 755 "$BUILD_DIR/DEBIAN/postrm"

# Control file
cat <<EOF > "$BUILD_DIR/DEBIAN/control"
Package: $PACKAGE_NAME
Version: $DEB_VERSION
Section: x11
Priority: optional
Architecture: $ARCH
Depends: libtqt3-mt, tdelibs14-trinity, twin-trinity | tdebase-trinity | tde-tdebase
Replaces: tdedecoration-twin-q4win10, tde-win-deco-q4win10-tde14
Conflicts: tdedecoration-twin-q4win10
Provides: tdedecoration-twin-q4win10
Maintainer: $MAINTAINER
Description: $DESCRIPTION
 Standalone Windows 10 & 11 style window decoration plugin for TDE Twin.
 Compatible with all TDE R14.1.x releases (R14.1.0 to R14.1.6+).
 Highly optimized build.
 Homepage: https://github.com/seb3773/tde-win-deco-Q4WIN10
EOF

echo "Building Debian package..."
dpkg-deb --build "$BUILD_DIR" "$DEB_NAME"

echo "=============================================="
echo "Success! Universal package created: $DEB_NAME"
ls -lh "$DEB_NAME"
echo "=============================================="

# Optional install test (pass --install / -i)
if [ "$INSTALL_FLAG" = "1" ] || [ "$INSTALL_AFTER_BUILD" = "1" ]; then
    echo "Installing $DEB_NAME..."
    sudo dpkg -i "$DEB_NAME"
    tdebuildsycoca >/dev/null 2>&1 || true
    echo "Done! You can select Q4WIN10 in Trinity Control Center -> Window Decorations."
fi
