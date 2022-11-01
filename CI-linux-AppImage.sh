#!/bin/bash

set -o verbose

PATH=/home/zaraka/Qt/5.15.2/gcc_64/bin/:$PATH

# Check versions
qmake -v
cmake --version
pandoc --version

# Build TB

mkdir buildAppImage
cd buildAppImage
cmake .. -DCMAKE_PREFIX_PATH="cmake/packages" -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Werror" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--fatal-warnings" -DTB_SUPPRESS_PCH=1 || exit 1
#cmake --build . --config Release || exit 1
make -j 24
make install DESTDIR=AppDir

# not needed QMAKE=$$QMAKE_QMAKE QML_SOURCES_PATHS=$$QML_IMPORT_PATH
# need to be resolved VERSION=$${VERSION_MAJOR}.$${VERSION_MINOR}-$${VERSION_BUILD}
# not needed LD_LIBRARY_PATH=$$LD_LIBRARY_PATH:$${PWD}/Dependencies/NDI/lib:/usr/lib64/samba/

# resolve how to deploy desktop file and icons
mkdir -p AppDir/usr/share/applications/
cp ../app/resources/linux/trenchbroom.desktop AppDir/usr/share/applications/
mkdir -p AppDir/usr/share/hicolor/256x256/apps/
cp ../app/resources/linux/icons/icon_256.png AppDir/usr/share/hicolor/256x256/apps/trenchbroom.png

linuxdeploy-x86_64.AppImage --appdir AppDir -e "AppDir/usr/bin/trenchbroom" -i "AppDir/usr/share/hicolor/256x256/apps/trenchbroom.png" -d "AppDir/usr/share/applications/trenchbroom.desktop" --plugin qt --output appimage
