#include "Octree.hpp"
#include "binaryrw.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[])
{
	if(argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " FILE_IN FILE_OUT" << std::endl;
		exit(EXIT_SUCCESS);
	}

	std::string file = split(argv[1], ':')[0];
	std::string coords = split(argv[1], ':')[1];
	Octree octree(readHDF5(file, coords.c_str()));
	std::ofstream f(argv[2], std::ios_base::out | std::ios_base::binary);
	write(f, octree);
	f.close();

	return EXIT_SUCCESS;
}
