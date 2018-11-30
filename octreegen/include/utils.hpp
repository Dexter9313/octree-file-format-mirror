#include <cfloat>
#include <hdf5/serial/hdf5.h>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(std::string const& str, char c = ' ');
std::string join(std::vector<std::string> const& strs, char c = ' ');
std::vector<float> generateVertices(unsigned int number, unsigned int seed);
std::vector<float> readHDF5(std::string const& path, const char* parttype);
