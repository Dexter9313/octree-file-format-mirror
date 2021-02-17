/*
    Copyright (C) 2018 Florian Cabot <florian.cabot@epfl.ch>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <fstream>
#include <future>
#include <iostream>
#include <vector>

#include <liboctree/Octree.hpp>
#include <liboctree/binaryrw.hpp>

#include "utils.hpp"

void man(const char* argv_0)
{
	std::cout << "Usage: " << std::endl
	          << "\t" << argv_0 << " FILES_IN:DATASET_COORD_PATH[:DATASET_RADIUS_PATH[:DATASET_LUM_PATH]] FILE_OUT" << std::endl
	          << "\t" << argv_0 << " FILES_IN:DATASET_COORD_PATH:DATASET_R_COLOR_PATH:DATASET_G_COLOR_PATH:DATASET_B_COLOR_PATH FILE_OUT" << std::endl
	          << "\t" << argv_0 << " PARTICLES_NUMBER FILE_OUT" << std::endl
	          << "\t" << argv_0 << " --update OCTREE_FILE_IN FILE_OUT" << std::endl
	          << "\n\t" << "FILES_IN are a set of paths separated by spaces (don't forget the quotes). Wildcards are supported."
	          << "\n\t" << "The --update option will only read then write a previously generated octree file, effectively updating its format to the current octreegen version default format." << std::endl;
	std::cout << "\nExamples: " << std::endl
	          << "\t"
	          << "To read gaz data coordinates and luminosity within snapshot.&ast;.hdf5 files (will be expanded as \"snapshot.0.hdf5 snapshot.1.hdf5\" for example) in "
	          << std::endl
	          << "\tgroup /PartType0 and write the corresponding octree in "
	          << std::endl
	          << "\tthe gaz.octree file :" << std::endl
	          << "\t" << argv_0
	          << " snapshot.*.hdf5:/PartType0/Coordinates::/PartType0/Luminosities gaz.octree" << std::endl
			  << "\twhich is equivalent to :" << std::endl
	          << "\t" << argv_0
	          << " \"snapshot.0.hdf5 snapshot.1.hdf5\":/PartType0/Coordinates::/PartType0/Luminosities gaz.octree" << std::endl
	          << std::endl
	          << "\t"
	          << "To generate 1 million uniformly random particles and "
	          << std::endl
	          << "\twrite the corresponding octree in the random.octree file "
	             ":"
	          << std::endl
	          << "\t" << argv_0 << " 1000000 random.octree" << std::endl;
}

int main(int argc, char* argv[])
{
	Octree octree;
	std::string output;
	if(argc < 3)
	{
		man(argv[0]);
		return EXIT_SUCCESS;
	}
	// update
	if(argc == 4 && argv[1] == std::string("--update"))
	{
		output = argv[3];
		std::cout << "Loading octree structure..." << std::endl;
		std::fflush(stdout);
		int64_t filesize(0);
		{
			std::ifstream in(argv[2], std::ifstream::ate | std::ifstream::binary);
			filesize = in.tellg();
		}
		std::ifstream in;
		in.open(argv[2], std::fstream::in | std::fstream::binary);

		// Init tree with progress bar
		int64_t cursor(in.tellg());
		int64_t size;
		brw::read(in, size);
		in.seekg(cursor);
		size *= -1;

		auto future = std::async(std::launch::async, &initOctree, &octree, &in);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		Octree::showProgress(0.f);
		while(future.wait_for(std::chrono::duration<int, std::milli>(100))
			  != std::future_status::ready)
		{
			float p(((float)in.tellg()) / size);
			if(0.f < p && p < 1.f)
			{
				Octree::showProgress(p);
			}
		}
		Octree::showProgress(1.f);
		std::cout << "Loading octree data..." << std::endl;

		future = std::async(std::launch::async, &readData, &octree, &in);
		Octree::showProgress(0.f);
		while(future.wait_for(std::chrono::duration<int, std::milli>(100))
			  != std::future_status::ready)
		{
			float p(((float)in.tellg()) / filesize);
			if(0.f < p && p < 1.f)
			{
				Octree::showProgress(p);
			}
		}
		Octree::showProgress(1.f);
	}
	else
	{
		output = argv[2];
		auto s(split(argv[1], ':'));
		if(s.size() >= 2 && s.size() <= 4)
		{
			Octree::Flags flags(Octree::Flags::NORMALIZED_NODES);
			std::string file   = s[0];
			std::string coords = s[1];
			std::string radius = "";
			if(s.size() >= 3)
			{
				radius = s[2];
				flags |= Octree::Flags::STORE_RADIUS;
			}
			std::string luminosity = "";
			if(s.size() >= 4)
			{
				luminosity = s[3];
				flags |= Octree::Flags::STORE_LUMINOSITY;
			}
			octree.setFlags(flags);
			std::vector<float> v;
			try
			{
				v = readHDF5(file, coords.c_str(), radius.c_str(), luminosity.c_str());
			}
			catch(std::string s)
			{
				std::cerr << "Error while reading HDF5 file(s) :" << std::endl;
				std::cerr << s << std::endl;
				return EXIT_FAILURE;
			}
			std::cout << "Constructing octree :" << std::endl;
			Octree::showProgress(0.f);
			octree.init(v);
		}
		else if(s.size() == 5)
		{
			Octree::Flags flags(Octree::Flags::NORMALIZED_NODES | Octree::Flags::STORE_COLOR);
			std::string file   = s[0];
			std::string coords = s[1];
			std::string r = s[2];
			std::string g = s[3];
			std::string b = s[4];
			octree.setFlags(flags);
			std::vector<float> v;
			try
			{
				v = readHDF5(file, coords.c_str(), r.c_str(), g.c_str(), b.c_str());
			}
			catch(std::string s)
			{
				std::cerr << "Error while reading HDF5 file(s) :" << std::endl;
				std::cerr << s << std::endl;
				return EXIT_FAILURE;
			}
			std::cout << "Constructing octree :" << std::endl;
			Octree::showProgress(0.f);
			octree.init(v);
		}
		else
		{
			size_t numberOfVertices;
			try
			{
				numberOfVertices = std::stoi(argv[1]);
			}
			catch(const std::invalid_argument& e)
			{
				std::cerr << "Invalid argument : " << argv[1] << std::endl;
				man(argv[0]);
				return EXIT_FAILURE;
			}
			std::vector<float> v(generateVertices(numberOfVertices, time(NULL)));
			std::cout << "Constructing octree :" << std::endl;
			Octree::showProgress(0.f);
			octree.init(v);
		}
	}
	std::ofstream f(output, std::ios_base::out | std::ios_base::binary);
	std::cout << "Writing octree to output file :" << std::endl;
	Octree::showProgress(0.f);
	write(f, octree);
	Octree::showProgress(1.f);
	f.close();
	std::cout << "Conversion successfull !" << std::endl;
	return EXIT_SUCCESS;
}
