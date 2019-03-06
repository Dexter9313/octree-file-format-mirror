# liboctree

Library that can build octrees, or read/write them from/to *.octree* files used by [VIRUP](https://gitlab.com/Dexter9313/virup).

## Requirements

### Running the tool

Everything should be self-contained.

### Building from source

* A C++ compiler (g++ for example)
* CMake

## Installation

### Ubuntu/Debian

Simply install the following deb package :

Download (.deb) : [liboctree 1.2.2](https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.2.2/raw/liboctree-1.2.2-linux_amd64.deb?job=pack:liboctree)

### Windows

You will find the necessary headers and libs for Windows on GitHub at [octree-file-format-mirror Releases](https://github.com/Dexter9313/octree-file-format-mirror/releases).

### Build from source

First install the required libraries :

	sudo apt-get update
	sudo apt-get install build-essential cmake

Then clone this repository. We now suppose the root directory of the repository is stored in the $OCTREE_ROOT_DIR variable.

	cd $OCTREE_ROOT_DIR
	cd liboctree
	mkdir build && cd build
	cmake ..
	make
	sudo make install

Optionally, you can generate a deb package to make installation managing easier if you are on a debian-based system. The package name will be "octreegen".

	cd $OCTREE_ROOT_DIR
	cd liboctree
	mkdir build && cd build
	cmake ..
	make package
	sudo dpkg -i ./*.deb

## Documentation

To use this library, please read the documentation : [https://dexter9313.gitlab.io/octree-file-format/](https://dexter9313.gitlab.io/octree-file-format/).

## Uninstall

If the deb method for installation was used :

	sudo apt-get autoremove liboctree

If the make install method for installation was used, uninstallation can only be done if the build directory has been left untouched since installation :

	cd $OCTREE_ROOT_DIR
	cd liboctree/build
	sudo make uninstall
