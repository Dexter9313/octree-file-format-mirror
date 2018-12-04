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
