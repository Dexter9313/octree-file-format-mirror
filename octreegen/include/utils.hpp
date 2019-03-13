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

#include <cfloat>
#include <hdf5/serial/hdf5.h>
#include <iostream>
#include <string>
#include <vector>

std::vector<std::string> split(std::string const& str, char c = ' ');
std::string join(std::vector<std::string> const& strs, char c = ' ');
std::vector<float> generateVertices(unsigned int number, unsigned int seed);
std::vector<float> readHDF5(std::string const& path, const char* parttype);
void showProgress(float progress);
