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

#ifdef _WIN32
#include <Windows.h>
void sleepOneSec() { Sleep(1000);}
#else
#include <unistd.h>
void sleepOneSec() { usleep(999999);}
#endif

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

std::vector<float> generateVertices(size_t number, unsigned int seed, unsigned int dimPerVertex)
{
	std::vector<float> vertices;
	vertices.reserve(dimPerVertex * number);

	srand(seed);

	for(size_t i(0); i < dimPerVertex * number; ++i)
	{
		vertices.push_back((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)));
	}

	return vertices;
}

std::vector<std::string> glob(std::string const& filePath)
{
	std::vector<std::string> result;
	glob_t globbuf;
	int err = glob(filePath.c_str(), 0, NULL, &globbuf);
	if(err == 0)
	{
		for(size_t i = 0; i < globbuf.gl_pathc; i++)
		{
			result.emplace_back(globbuf.gl_pathv[i]);
		}
		globfree(&globbuf);
	}

	return result;
}

std::vector<std::string> parseFiles(std::string const& filePath)
{
	std::vector<std::string> result;

	for(auto s : split(filePath))
	{
		std::vector<std::string> gl(glob(s));
		if(gl.empty())
		{
			throw(s + " : No such file or directory.");
		}
		for(auto g : gl)
		{
			result.push_back(g);
		}
	}
	std::unordered_set<std::string> s(result.begin(), result.end());
	result.assign(s.begin(), s.end());
	return result;
}

void readOctreeStructureOnly(std::string const& octreeFilePath, Octree& octree)
{
		std::cout << "Loading octree structure..." << std::endl;
		std::fflush(stdout);
		std::ifstream in;
		in.open(octreeFilePath, std::fstream::in | std::fstream::binary);

		// Init tree with progress bar
		int64_t cursor(in.tellg());
		int64_t size;
		brw::read(in, size);
		in.seekg(cursor);
		size *= -1;

		auto future = std::async(std::launch::async, &initOctree, &octree, &in);
		sleepOneSec();
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
}

void readOctreeContentOnly(std::string const& octreeFilePath, Octree& octree)
{
		std::cout << "Loading octree data..." << std::endl;
		std::fflush(stdout);
		int64_t filesize(0);
		{
			std::ifstream in(octreeFilePath, std::ifstream::ate | std::ifstream::binary);
			filesize = in.tellg();
		}
		std::ifstream in;
		in.open(octreeFilePath, std::fstream::in | std::fstream::binary);

		auto future = std::async(std::launch::async, &readData, &octree, &in);
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

void readOctreeContent(std::string const& octreeFilePath, Octree& octree)
{
	readOctreeStructureOnly(octreeFilePath, octree);
	readOctreeContentOnly(octreeFilePath, octree);
}

size_t totalNumberOfVertices(std::vector<std::string> const& filesPaths,
                             const char* datasetPath)
{
	size_t result(0);
	bool ok(true);
	for(auto filePath : filesPaths)
	{
		hid_t space;
		hsize_t dims[2];
		dims[1] = 1;

		hid_t hdf5_file
		    = H5Fopen(filePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
		if(hdf5_file < 0)
		{
			std::cerr << filePath + " isn't a valid HDF5 file." << std::endl;
			ok = false;
			continue;
		}

		hid_t hdf5_dataset = H5Dopen(hdf5_file, datasetPath, H5P_DEFAULT);
		if(hdf5_dataset < 0)
		{
			H5Fclose(hdf5_file);
			std::cerr << filePath + ":" + datasetPath
			                 + " isn't a valid HDF5 Dataset path."
			          << std::endl;
			ok = false;
			continue;
		}

		space = H5Dget_space(hdf5_dataset);
		if(space < 0)
		{
			H5Dclose(hdf5_dataset);
			H5Fclose(hdf5_file);
			std::cout << "Cannot get space of " << filePath + ":" + datasetPath
			          << std::endl;
			ok = false;
			continue;
		}

		if(H5Sget_simple_extent_dims(space, dims, NULL) < 0)
		{
			H5Dclose(hdf5_dataset);
			H5Fclose(hdf5_file);
			std::cerr << "Cannot get simple extent dimensions of "
			          << filePath + ":" + datasetPath;
			ok = false;
			continue;
		}

		result += dims[0];
		H5Dclose(hdf5_dataset);
		H5Fclose(hdf5_file);
	}
	if(!ok)
	{
		throw(std::string("There were problems with provided files."));
	}
	return result;
}

size_t readHDF5Dataset(std::string const& filePath, const char* datasetPath,
                       std::vector<float>& result, size_t offset,
                       unsigned int stride)
{
	hid_t hdf5_file;
	hid_t hdf5_dataset;
	hsize_t dims[2];
	dims[1] = 1;

	hid_t space, memspace;

	hdf5_file = H5Fopen(filePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
	if(hdf5_file < 0)
	{
		throw(filePath + " isn't a valid HDF5 file.");
	}

	hdf5_dataset = H5Dopen(hdf5_file, datasetPath, H5P_DEFAULT);
	if(hdf5_dataset < 0)
	{
		H5Fclose(hdf5_file);
		throw(filePath + ":" + datasetPath
		      + " isn't a valid HDF5 Dataset path.");
	}

	space = H5Dget_space(hdf5_dataset);
	if(space < 0)
	{
		H5Dclose(hdf5_dataset);
		H5Fclose(hdf5_file);
		throw(std::string("Cannot get space of ") + filePath + ":"
		      + datasetPath);
	}

	if(H5Sget_simple_extent_dims(space, dims, NULL) < 0)
	{
		H5Dclose(hdf5_dataset);
		H5Fclose(hdf5_file);
		throw(std::string("Cannot get simple extent dimensions of ") + filePath
		      + ":" + datasetPath);
	}

	hsize_t totalSize(dims[0] * stride);
	if(result.size() == 0)
	{
		result.resize(totalSize);
	}

	totalSize = result.size();
	memspace  = H5Screate_simple(1, &totalSize, NULL);
	if(memspace < 0)
	{
		H5Dclose(hdf5_dataset);
		H5Fclose(hdf5_file);
		std::ostringstream oss("Cannot create a simple dataspace of size ");
		oss << totalSize;
		throw(oss.str());
	}

	hsize_t hoffset(offset), hstride(stride);
	if(H5Sselect_hyperslab(memspace, H5S_SELECT_SET, &hoffset, &hstride,
	                       &dims[0], &dims[1])
	   < 0)
	{
		H5Dclose(hdf5_dataset);
		H5Fclose(hdf5_file);
		std::ostringstream oss("Cannot select an hyperslab of offset ");
		oss << offset;
		oss << ", stride ";
		oss << stride;
		oss << ", count ";
		oss << dims[0];
		oss << " and block ";
		oss << dims[1];
		oss << " in " << filePath << ":" << datasetPath;
		throw(oss.str());
	}

	if(H5Dread(hdf5_dataset, H5T_NATIVE_FLOAT, memspace, H5S_ALL, H5P_DEFAULT,
	           &result[0])
	   < 0)
	{
		H5Dclose(hdf5_dataset);
		H5Fclose(hdf5_file);
		throw(filePath + ":" + datasetPath + " : Cannot read content...");
	}

	H5Dclose(hdf5_dataset);

	H5Fclose(hdf5_file);

	return dims[0];
}

std::vector<float> readHDF5(std::string const& filePath,
                            const char* pathToCoordinates,
                            const char* pathToRadius,
                            const char* pathToLuminosity)
{
	std::vector<std::string> files(parseFiles(filePath));

	std::cout << "Reading file(s) :" << std::endl;
	for(auto f : files)
	{
		std::cout << f << std::endl;
	}
	std::cout << std::endl;

	size_t verticesNumber
	    = totalNumberOfVertices(parseFiles(filePath), pathToCoordinates);
	std::cout << "Total number of vertices to read : " << verticesNumber
	          << std::endl
	          << std::endl;

	bool radius(pathToRadius[0] != '\0');
	bool luminosity(pathToLuminosity[0] != '\0');
	unsigned int stride(3);
	size_t fileOffset(0), offset(0);
	if(radius)
		stride++;
	if(luminosity)
		stride++;

	std::vector<float> result(verticesNumber * stride);
	std::cout << "Reading HDF5 data :" << std::endl;
	for(auto file : files)
	{
		std::cout << file << std::endl;

		offset = 0;
		size_t fileOffset_back(fileOffset);

		Octree::showProgress(0.f);
		fileOffset += stride
		              * readHDF5Dataset(file, pathToCoordinates, result,
		                                fileOffset_back + offset, stride);
		offset += 3;
		Octree::showProgress(0.333f);
		// radius
		if(radius)
		{
			readHDF5Dataset(file, pathToRadius, result,
			                fileOffset_back + offset, stride);
			offset++;
		}
		Octree::showProgress(0.666f);
		if(luminosity)
		{
			readHDF5Dataset(file, pathToLuminosity, result,
			                fileOffset_back + offset, stride);
			offset++;
		}
		Octree::showProgress(1.f);
	}

	std::cout << "Loaded from file : " << result.size() / stride << " points"
	          << std::endl;
	return result;
}

std::vector<float> readHDF5(std::string const& filePath,
                            const char* pathToCoordinates, const char* pathToR,
                            const char* pathToG, const char* pathToB)
{
	std::vector<std::string> files(parseFiles(filePath));

	std::cout << "Reading file(s) :" << std::endl;
	for(auto f : files)
	{
		std::cout << f << std::endl;
	}
	std::cout << std::endl;

	size_t verticesNumber
	    = totalNumberOfVertices(parseFiles(filePath), pathToCoordinates);
	std::cout << "Total number of vertices to read : " << verticesNumber
	          << std::endl
	          << std::endl;

	unsigned int stride(6);
	size_t fileOffset(0), offset(0);

	std::vector<float> result(verticesNumber * stride);
	std::cout << "Reading HDF5 data :" << std::endl;
	for(auto file : files)
	{
		std::cout << file << std::endl;

		offset = 0;
		size_t fileOffset_back(fileOffset);

		Octree::showProgress(0.f);
		fileOffset += stride
		              * readHDF5Dataset(file, pathToCoordinates, result,
		                                fileOffset_back + offset, stride);
		offset += 3;
		Octree::showProgress(0.25f);
		readHDF5Dataset(file, pathToR, result, fileOffset_back + offset,
		                stride);
		offset++;
		Octree::showProgress(0.5f);
		readHDF5Dataset(file, pathToG, result,
		                fileOffset_back + offset, stride);
		offset++;
		Octree::showProgress(0.75f);
		readHDF5Dataset(file, pathToB, result,
		                fileOffset_back + offset, stride);
		offset++;
		Octree::showProgress(1.f);
	}

	std::cout << "Loaded from file : " << result.size() / stride << " points"
	          << std::endl;
	return result;
}

void initOctree(Octree* octree, std::istream* file)
{
	octree->init(*file);
}

void readData(Octree* octree, std::istream* file)
{
	octree->readData(*file);
}
