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

#ifndef UTILS
#define UTILS

#include <cfloat>
#include <hdf5.h>
#include <iostream>
#include <liboctree/Octree.hpp>
#include <string>
#include <unordered_set>
#include <vector>
#include <glob.h>
#include <future>



std::vector<std::string> split(std::string const& str, char c = ' ');
std::string join(std::vector<std::string> const& strs, char c = ' ');
std::vector<float> generateVertices(size_t number, unsigned int seed, unsigned int dimPerVertex = 3);
std::vector<std::string> glob(std::string const& filePath);
std::vector<std::string> parseFiles(std::string const& filePath);
void readOctreeStructureOnly(std::string const& octreeFilePath, Octree& octree);
void readOctreeContentOnly(std::string const& octreeFilePath, Octree& octree);
void readOctreeContent(std::string const& octreeFilePath, Octree& octree);
size_t totalNumberOfVertices(std::vector<std::string> const& filesPaths, const char* datasetPath);
size_t readHDF5Dataset(std::string const& filePath, const char* datasetPath, std::vector<float>& result, size_t offset = 0, unsigned int stride = 5);
std::vector<float> readHDF5(std::string const& filePath, const char* pathToCoordinates, const char* pathToRadius = "", const char* pathToLuminosity = "");
std::vector<float> readHDF5(std::string const& filePath, const char* pathToCoordinates, const char* pathToR, const char* pathToG, const char* pathToB);

void initOctree(Octree* octree, std::istream* file);
void readData(Octree* octree, std::istream* file);

#endif
