#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT_DIR/build"
DIST_DIR="$ROOT_DIR/dist"
APPDIR="$ROOT_DIR/AppDir"
QT_PREFIX="${QT_PREFIX:-/home/g0x4d1cf/Qt/6.10.2/gcc_64}"
QMAKE_BIN="$QT_PREFIX/bin/qmake"

mkdir -p "$DIST_DIR"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/scalable/apps"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR" -DCMAKE_PREFIX_PATH="$QT_PREFIX"
cmake --build "$BUILD_DIR" -j

cp "$BUILD_DIR/astreus-package-downloader" "$APPDIR/usr/bin/"
cp "$ROOT_DIR/packaging/astreus.desktop" "$APPDIR/usr/share/applications/"
cp "$ROOT_DIR/resources/icons/astreus.svg" "$APPDIR/usr/share/icons/hicolor/scalable/apps/"
cp "$ROOT_DIR/resources/icons/astreus.svg" "$APPDIR/astreus.svg"
cp "$ROOT_DIR/packaging/astreus.desktop" "$APPDIR/astreus.desktop"

export QMAKE="$QMAKE_BIN"
export QML_SOURCES_PATHS="$ROOT_DIR/qml"
export EXTRA_QT_PLUGINS="iconengines,imageformats,platformthemes/libqgtk3.so"
export APPIMAGE_EXTRACT_AND_RUN=1

"$ROOT_DIR/tools/linuxdeploy-x86_64.AppImage" \
  --appdir "$APPDIR" \
  -d "$ROOT_DIR/packaging/astreus.desktop" \
  -i "$ROOT_DIR/resources/icons/astreus.svg" \
  --executable "$APPDIR/usr/bin/astreus-package-downloader" \
  --plugin qt

"$ROOT_DIR/tools/appimagetool-x86_64.AppImage" "$APPDIR" "$DIST_DIR/Astreus-Package-Downloader-x86_64-v0.1.1.AppImage"
chmod +x "$DIST_DIR/Astreus-Package-Downloader-x86_64-v0.1.1.AppImage"

sha256sum "$DIST_DIR/Astreus-Package-Downloader-x86_64-v0.1.1.AppImage" > "$DIST_DIR/SHA256SUMS-v0.1.1.txt"

echo "AppImage created: $DIST_DIR/Astreus-Package-Downloader-x86_64-v0.1.1.AppImage"
