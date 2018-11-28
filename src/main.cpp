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

	Octree octree(readHDF5(argv[1], "/PartType0"));
	/*Octree octree(
	        {
	            1, 1, 1,
	            -1, 1, 1,
	            1, -1, 1,
	            1, 1, -1,
	            -1, -1, 1,
	            1, -1, -1,
	            -1, 1, -1,
	            -1, -1, -1,
	        }
	        );*/
	// for(long c : octree.getCompactData())
	//	std::cout << c << std::endl;

	/*std::ifstream f(argv[1], std::ios_base::in | std::ios_base::binary);
	uint32_t null;
	long nulll;
	brw::read(f, null);
	brw::read(f, nulll);
	Octree octree(f);
	std::cout << "Begin big read" << std::endl;
	octree.readData(f);
	std::cout << "End big read" << std::endl;
	system("sleep 5");
	//octree.debug();*/

	std::ofstream f(argv[2], std::ios_base::out | std::ios_base::binary);
	write(f, octree);
	f.close();

	return EXIT_SUCCESS;
}
