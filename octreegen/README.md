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
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.10.0/raw/liboctree-1.10.0-linux_amd64.deb?job=pack:liboctree
	sudo dpkg -i ./*.deb

Then simply install the following deb package :

Download (.deb) : [octreegen 1.15.0](https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.15.0/raw/octreegen-1.15.0-linux_amd64.deb?job=pack:octreegen)

### Build from source

First install the required libraries :

	sudo apt-get update
	sudo apt-get install build-essential cmake libhdf5-serial-dev
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.15.0/raw/liboctree-1.15.0-linux_amd64.deb?job=pack:liboctree
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

	octreegen FILES_IN:DATASET_COORD_PATH[:DATASET_RADIUS_PATH[:DATASET_LUM_PATH]] FILE_OUT
	octreegen FILES_IN:DATASET_COORD_PATH:DATASET_R_COLOR_PATH:DATASET_G_COLOR_PATH:DATASET_B_COLOR_PATH FILE_OUT
	octreegen PARTICLES_NUMBER FILE_OUT
	octreegen --update OCTREE_FILE_IN FILE_OUT
	octreegen --subsample RATE OCTREE_FILE_IN FILE_OUT
	octreegen --merge OCTREE_FILE_IN1 OCTREE_FILE_IN2 FILE_OUT

	FILES_IN are a set of paths separated by spaces (don't forget the quotes). Wildcards are supported.
	The --update option will only read then write a previously generated octree file, effectively updating its format to the current octreegen version default format.

	The --subsample option will take a ratio RATE of the OCTREE_FILE_IN data to write it in FILE_OUT. (ex: To halve the data, put RATE as 0.5. To take one vertex out of 4, put RATE as 0.25.)

### Examples

To read gaz data coordinates and luminosity within snapshot.&ast;.hdf5 files (will be expanded as "snapshot.0.hdf5 snapshot.1.hdf5" for example) in group /PartType0 and write the corresponding octree in the gaz.octree file :

	octreegen snapshot.*.hdf5:/PartType0/Coordinates::/PartType0/Luminosities gaz.octree

which is equivalent to :

	octreegen "snapshot.0.hdf5 snapshot.1.hdf5":/PartType0/Coordinates::/PartType0/Luminosities gaz.octree

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
