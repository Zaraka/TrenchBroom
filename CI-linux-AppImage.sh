#!/bin/bash

set -o verbose

# Check versions
qmake -v
cmake --version
pandoc --version

# Build TB

mkdir buildAppImage
cd buildAppImage
cmake .. -DCMAKE_PREFIX_PATH="cmake/packages" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" -DTB_SUPPRESS_PCH=1 || exit 1
#cmake --build . --config Release || exit 1
make -j
make install DESTDIR=AppDir

# TODO: need to be resolved VERSION=??

# TODO:
mkdir -p AppDir/usr/share/applications/
cp ../app/resources/linux/trenchbroom.desktop AppDir/usr/share/applications/
mkdir -p AppDir/usr/share/hicolor/256x256/apps/
cp ../app/resources/linux/icons/icon_256.png AppDir/usr/share/hicolor/256x256/apps/trenchbroom.png

linuxdeploy-x86_64.AppImage --appdir AppDir -e "AppDir/usr/bin/trenchbroom" -i "AppDir/usr/share/hicolor/256x256/apps/trenchbroom.png" -d "AppDir/usr/share/applications/trenchbroom.desktop" --plugin qt --output appimage
