# octreegen

Generates an octree file either from an HDF5 file or from a number to random particles to generate.

## Requirements

### Running the tool

* [liboctree](https://gitlab.com/Dexter9313/octree-file-format/blob/master/liboctree/)
* LibHDF5

### Building from source

* A C++ compiler (g++ for example)
* CMake
* [liboctree](https://gitlab.com/Dexter9313/octree-file-format/blob/master/liboctree/)
* LibHDF5

## Installation

### Ubuntu/Debian

First install the required libraries :

	sudo apt-get update
	sudo apt-get install libhdf5-100
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.2.2/raw/liboctree-1.2.2-linux_amd64.deb?job=pack:liboctree
	sudo dpkg -i ./*.deb

Then simply install the following deb package :

Download (.deb) : [octreegen 1.2.2](https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.2.2/raw/octreegen-1.2.2-linux_amd64.deb?job=pack:octreegen)

### Build from source

First install the required libraries :

	sudo apt-get update
	sudo apt-get install build-essential cmake libhdf5-serial-dev
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.2.2/raw/liboctree-1.2.2-linux_amd64.deb?job=pack:liboctree
	sudo dpkg -i ./*.deb

Then clone this repository. We now suppose the root directory of the repository is stored in the $OCTREE_ROOT_DIR variable.

	cd $OCTREE_ROOT_DIR
	cd octreegen
	mkdir build && cd build
	cmake ..
	make
	sudo make install

Optionally, you can generate a deb package to make installation managing easier if you are on a debian-based system. The package name will be "octreegen".

	cd $OCTREE_ROOT_DIR
	cd octreegen
	mkdir build && cd build
	cmake ..
	make package
	sudo dpkg -i ./*.deb

## Usage

	octreegen FILE_IN:DATASET_PATH FILE_OUT

	octreegen PARTICLES_NUMBER FILE_OUT

### Examples

To read gaz data coordinates within snapshot.hdf5 in 
group /PartType0 and write the corresponding octree in 
the gaz.octree file :

	octreegen snapshot.hdf5:/PartType0/Coordinates gaz.octree

To generate 1 million uniformly random particles and 
write the corresponding octree in the random.octree file :

	octreegen 1000000 random.octree

## Uninstall

If the deb method for installation was used :

	sudo apt-get autoremove octreegen

If the make install method for installation was used, uninstallation can only be done if the build directory has been left untouched since installation :

	cd $OCTREE_ROOT_DIR
	cd octreegen/build
	sudo make uninstall
