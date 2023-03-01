#!/bin/bash

# actually 13.10

set -x

sed -i -re 's/([a-z]{2}\.)?archive.ubuntu.com|security.ubuntu.com/old-releases.ubuntu.com/g' /etc/apt/sources.list

cd /opt
cp /project/octreegen/ubuntu-13.10-deps/cmake* .
chmod +x ./cmake-3.25.0-rc4-linux-x86_64.sh
./cmake-3.25.0-rc4-linux-x86_64.sh --skip-license --prefix=.
cd /usr/bin
ln -s /opt/bin/cmake
cmake --version
cd /project

apt-get update
apt-get install -y build-essential libhdf5-serial-dev wget file git

cd liboctree
rm -rf build || true
mkdir build ; cd build
cmake ..
make -j$(nproc) install

cd ..
rm -rf build

cd ../octreegen
./make-appimage.sh
