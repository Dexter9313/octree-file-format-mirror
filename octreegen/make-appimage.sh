
set -e

rm -rf build || true
mkdir build
cd build

cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release
make install DESTDIR=AppDir -j $(nproc)

# now, build AppImage using linuxdeploy
cp ../ubuntu-13.10-deps/linuxdeploy-x86_64.AppImage .

# make them executable and extract them (to make them compatible with docker
chmod +x linuxdeploy*.AppImage
./linuxdeploy-x86_64.AppImage --appimage-extract
rm linuxdeploy*.AppImage # important ! else the AppImage of the Qt plugin will be used (and will fail on docker)
# initialize AppDir, bundle shared libraries for QtQuickApp, use Qt plugin to bundle additional resources, and build AppImage, all in one single command
VERSION=0.0 ./squashfs-root/AppRun --appdir AppDir --output appimage -d ../octreegen.desktop -i ../icon.png
