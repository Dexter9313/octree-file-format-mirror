# octreegen-gui

A user-friendly GUI interface for octreegen.

## Requirements

### Running the tool

* [octreegen](https://gitlab.com/Dexter9313/octree-file-format/blob/master/octreegen/)
* LibHDF5
* Qt5 Widgets

### Building from source

* A C++ compiler (g++ for example)
* CMake
* LibHDF5
* Qt5 Widgets

## Installation

### Ubuntu/Debian

First install the required libraries :

	sudo apt-get update
	sudo apt-get install libhdf5-100 libqt5widgets5
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.10.0/raw/octreegen-1.10.0-linux_amd64.deb?job=pack:octreegen
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.10.0/raw/liboctree-1.10.0-linux_amd64.deb?job=pack:liboctree
	sudo dpkg -i ./*.deb

Then simply install the following deb package :

Download (.deb) : [octreegen-gui 1.16.0](https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.16.0/raw/octreegen-gui-1.16.0-linux_amd64.deb?job=pack:octreegen-gui)

### Build from source

First install the required libraries :

	sudo apt-get update
	sudo apt-get install build-essential cmake libhdf5-serial-dev qtbase5-dev

Then clone this repository. We now suppose the root directory of the repository is stored in the $OCTREE_ROOT_DIR variable.

	cd $OCTREE_ROOT_DIR
	cd octreegen-gui
	mkdir build && cd build
	cmake ..
	make
	sudo make install

Optionally, you can generate a deb package to make installation managing easier if you are on a debian-based system. The package name will be "octreegen-gui".

	cd $OCTREE_ROOT_DIR
	cd octreegen-gui
	mkdir build && cd build
	cmake ..
	make package
	sudo dpkg -i ./*.deb

## Usage

Simply open an HDF5 file with File->Open then select a coordinates dataset within the datasets hierarchy (optionally radius and luminosity too). You can open several files at once if your data is spread out between multiple files. Then click the generate button and you are done when the loading bar reaches 100% !

## Uninstall

If the deb method for installation was used :

	sudo apt-get autoremove octreegen-gui

If the make install method for installation was used, uninstallation can only be done if the build directory has been left untouched since installation :

	cd $OCTREE_ROOT_DIR
	cd octreegen-gui/build
	sudo make uninstall
