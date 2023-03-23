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

Download (.deb) : [octreegen 1.16.0](https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.16.0/raw/octreegen-1.16.0-linux_amd64.deb?job=pack:octreegen)

### Build from source

First install the required libraries :

	sudo apt-get update
	sudo apt-get install build-essential cmake libhdf5-serial-dev
	wget --content-disposition https://gitlab.com/Dexter9313/octree-file-format/-/jobs/artifacts/1.16.0/raw/liboctree-1.16.0-linux_amd64.deb?job=pack:liboctree
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

	octreegen [-h|--help]
		Prints this help message.
	octreegen <COMMAND> [COMMAND-OPTIONS]

	Commands:
		octreegen info ...
			Prints informations from an existing octree file.
		octreegen generate...
			Generates an octree file given data from various sources.

		All commands have a [-h|-help] option to display their own help page.

### octreegen info

	octreegen info [-h|--help]
		Prints this help message.
	octreegen info <OCTREE-FILE>
		Prints informations about OCTREE_FILE. Cannot handle globbing or several files as an input.

### octreegen generate

	octreegen generate [-h|--help]
		Prints this help message.
	octreegen generate [INPUT-OPTIONS] <INPUT> --output [OUTPUT-OPTIONS] <OCTREE-FILE-OUT>
		Takes some input data and generates an octree written in OCTREE-FILE-OUT.

	INPUT OPTIONS:
		--sample-rate=<RATE> : resamples the input to only take RATE fraction particles (ex: --sample-rate=0.5 halves the input data).

	INPUT:
		Either of:
		--input-random <PARTICLES-NUMBER> [ADDITIONAL-DIMENSIONS] : specifies random data as particles, generates PARTICLES-NUMBER particles. Additional random dimensions can be specified as ADDITIONAL-DIMENSIONS :
			--add-radius
			--add-lum
			--add-rgb-lum
			--add-density
			--add-temperature
		--input-octree <OCTREE-FILES> : specifies octree file(s) as input (globbing works). If several files are specified, they must share the same flags (check flags using octreegen info).
		--input-hdf5 <HDF5-FILES> --coord-path=<COORD-DATASET-PATH> [ADDITIONAL-DATASET-PATHS] : specifies hdf5 file(s) as input (glob works). If several files are specified, they must share the same dataset path structure. COORD-DATASET-PATH is the 3D dataset path of particles coordinates. Additional variables dataset path can be specified as ADDITIONAL-DATASET-PATHS :
			--radius-path=<RADIUS-DATASET-PATH> : 1D dataset
			--lum-path=<LUM-DATASET-PATH> : total luminosity (1D dataset)
			--rgb-lum-path=<RGB-LUM-PATH> : luminosity per band (3D dataset)
			--density-path=<DENSITY-DATASET-PATH> : 1D dataset
			--temperature-path=<TEMPERATURE-DATASET-PATH> : 1D dataset

	OUTPUT-OPTIONS
		--disable-node-normalization : disables particles having coordinates in [0;1] relative to their node, which is on by default
		--max-particles-per-node=<MAX_PART_PER_NODE> : defines a particle number above which a node is split in 8 sub-nodes (and below which it becomes a leaf). MAX_PART_PER_NODE is 16000 by default.

### Examples

To read gaz data coordinates and luminosity within snapshot.\*.hdf5 files (will be expanded as "snapshot.0.hdf5 snapshot.1.hdf5" for example) in group /PartType0 and write the corresponding octree in the gaz.octree file :

	octreegen generate --input-hdf5 snapshot.\*.hdf5 --coord-path=/PartType0/Coordinates --lum-path=/PartType0/Luminosities --output gaz.octree

which is equivalent to :

	octreegen generate --input-hdf5 snapshot.0.hdf5 snapshot.1.hdf5 --coord-path=/PartType0/Coordinates --lum-path=/PartType0/Luminosities --output gaz.octree

To generate 1 million uniformly random particles and write the corresponding octree in the random.octree file :

	octreegen generate --input-random 1000000 --output random.octree

## Uninstall

If the deb method for installation was used :

	sudo apt-get autoremove octreegen

If the make install method for installation was used, uninstallation can only be done if the build directory has been left untouched since installation :

	cd $OCTREE_ROOT_DIR
	cd octreegen/build
	sudo make uninstall
