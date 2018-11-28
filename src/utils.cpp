#include "utils.hpp"

std::vector<float> generateVertices(unsigned int number, unsigned int seed)
{
	std::vector<float> vertices;
	vertices.reserve(3 * number);

	srand(seed);

	for(unsigned int i(0); i < 3 * number; ++i)
	{
		vertices.push_back(
		    2 * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		    - 1);
	}

	return vertices;
}

std::vector<float> readHDF5(std::string const& path, const char* parttype)
{
	hid_t hdf5_file, hdf5_grp[6];
	hid_t hdf5_dataset;
	hsize_t dims[2];

	hid_t space;
	float** rdata;
	// int status;

	hdf5_file    = H5Fopen(path.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	hdf5_grp[0]  = H5Gopen(hdf5_file, parttype, H5P_DEFAULT);
	hdf5_dataset = H5Dopen(hdf5_grp[0], "Coordinates", H5P_DEFAULT);
	space        = H5Dget_space(hdf5_dataset);
	H5Sget_simple_extent_dims(space, dims, NULL);

	std::vector<float> result(dims[0] * dims[1]);
	rdata    = new float*[dims[0]];
	rdata[0] = &result[0];
	for(unsigned int i = 1; i < (unsigned int) dims[0]; i++)
		rdata[i]       = rdata[0] + i * dims[1];

	/*status =*/H5Dread(hdf5_dataset, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL,
	                    H5P_DEFAULT, rdata[0]);

	// put everything in the unit cube
	for(unsigned int j(0); j < 3; ++j)
	{
		float max(0.f), min(FLT_MAX);
		for(unsigned int i(j); i < result.size(); i += 3)
		{
			if(result[i] > max)
				max = result[i];
			if(result[i] < min)
				min = result[i];
		}
		for(unsigned int i(j); i < result.size(); i += 3)
		{
			result[i] -= min;
			result[i] /= 0.5 * (max - min);
			result[i] -= 1;
		}
	}

	H5Fclose(hdf5_file);
	delete[] rdata;

	std::cout << "Loaded from file : " << result.size() / 3 << " points"
	          << std::endl;
	return result;
}
