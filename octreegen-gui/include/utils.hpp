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

#ifndef UTILS_H
#define UTILS_H

#include <hdf5/serial/hdf5.h>

#include <string>
#include <vector>

typedef H5O_type_t HDF5ObjectType;

struct HDF5Object
{
	std::vector<HDF5Object> links;
	std::string name;
	HDF5Object* parent;
	hid_t id;
	HDF5ObjectType type;
};

herr_t iterateCallback(hid_t loc_id, const char* name, const H5L_info_t* info,
                       void* operator_data);

HDF5Object readHDF5RootObject(std::string const& filePath);

#endif // UTILS_H
