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

#include "utils.hpp"

std::vector<std::string> split(std::string const& str, char c)
{
	// look from the end to use push_back (there is no "push_front")
	size_t p(str.rfind(c));
	if(p == std::string::npos)
		return {str};

	std::string left(str.substr(0, p)),
	    right(str.substr(p + 1, str.size() - p - 1));
	std::vector<std::string> res(split(left, c));
	res.push_back(right);
	return res;
}

std::string join(std::vector<std::string> const& strs, char c)
{
	std::string res("");
	for(std::string str : strs)
		res += str + c;
	res.pop_back();
	return res;
}

std::vector<float> generateVertices(size_t number, unsigned int seed)
{
	std::vector<float> vertices;
	vertices.reserve(3 * number);

	srand(seed);

	for(size_t i(0); i < 3 * number; ++i)
	{
		vertices.push_back(
		    2 * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		    - 1);
	}

	return vertices;
}

void readHDF5Dataset(std::string const& filePath, const char* datasetPath, std::vector<float>& result, unsigned int offset, unsigned int stride)
{
	(void) offset;
	hid_t hdf5_file;
	hid_t hdf5_dataset;
	hsize_t dims[2];
	dims[1] = 1;

	hid_t space, memspace;
	// int status;

	hdf5_file    = H5Fopen(filePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	hdf5_dataset = H5Dopen(hdf5_file, datasetPath, H5P_DEFAULT);
	space        = H5Dget_space(hdf5_dataset);
	H5Sget_simple_extent_dims(space, dims, NULL);

	hsize_t totalSize(dims[0] * stride);
	if(result.size() == 0)
	{
		result.resize(totalSize);
	}

	memspace = H5Screate_simple(1, &totalSize, NULL);

	hsize_t hoffset(offset), hstride(stride);
	H5Sselect_hyperslab(memspace, H5S_SELECT_SET, &hoffset, &hstride, &dims[0], &dims[1]);

	/*status =*/H5Dread(hdf5_dataset, H5T_NATIVE_FLOAT, memspace, H5S_ALL,
	                    H5P_DEFAULT, &result[0]);

	H5Dclose(hdf5_dataset);

	H5Fclose(hdf5_file);
}

std::vector<float> readHDF5(std::string const& filePath,
                            const char* pathToCoordinates,
                            const char* pathToRadius,
                            const char* pathToLuminosity)
{
	bool radius(pathToRadius[0] != '\0');
	bool luminosity(pathToLuminosity[0] != '\0');
	unsigned int offset(0), stride(3);
	if(radius)
		stride++;
	if(luminosity)
		stride++;

	std::vector<float> result;
	std::cout << "Reading HDF5 data :" << std::endl;
	Octree::showProgress(0.f);
	readHDF5Dataset(filePath, pathToCoordinates, result, offset, stride);
	offset += 3;
	Octree::showProgress(0.333f);
	// radius
	if(radius)
	{
		readHDF5Dataset(filePath, pathToRadius, result, offset, stride);
		offset++;
	}
	Octree::showProgress(0.666f);
	if(luminosity)
	{
		readHDF5Dataset(filePath, pathToLuminosity, result, offset, stride);
		offset++;
	}
	Octree::showProgress(1.f);

	std::cout << "Loaded from file : " << result.size() / stride << " points"
	          << std::endl;
	return result;
}
