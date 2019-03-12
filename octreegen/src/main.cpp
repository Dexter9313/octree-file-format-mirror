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
#include <iostream>
#include <vector>

#include <liboctree/Octree.hpp>
#include <liboctree/binaryrw.hpp>

#include "utils.hpp"

void man(const char* argv_0)
{
	std::cout << "Usage: " << std::endl
	          << "\t" << argv_0 << " FILE_IN:DATASET_PATH FILE_OUT" << std::endl
	          << "\t" << argv_0 << " PARTICLES_NUMBER FILE_OUT" << std::endl;
	std::cout << "Examples: " << std::endl
	          << "\t"
	          << "To read gaz data coordinates within snapshot.hdf5 in "
	          << std::endl
	          << "\tgroup /PartType0 and write the corresponding octree in "
	          << std::endl
	          << "\tthe gaz.octree file :" << std::endl
	          << "\t" << argv_0
	          << " snapshot.hdf5:/PartType0/Coordinates gaz.octree" << std::endl
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
	if(argc < 3)
	{
		man(argv[0]);
		return EXIT_SUCCESS;
	}
	else if(split(argv[1], ':').size() == 2)
	{
		std::string file   = split(argv[1], ':')[0];
		std::string coords = split(argv[1], ':')[1];
		std::vector<float> v(readHDF5(file, coords.c_str()));
		octree.init(v);
	}
	else
	{
		unsigned int numberOfVertices;
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
		octree.init(v);
	}
	std::cout << "Tree construction finished." << std::endl;
	std::ofstream f(argv[2], std::ios_base::out | std::ios_base::binary);
	write(f, octree);
	f.close();
	std::cout << "Tree writing finished." << std::endl;
	return EXIT_SUCCESS;
}
