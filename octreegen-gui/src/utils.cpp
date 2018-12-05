#include "utils.hpp"

herr_t iterateCallback(hid_t loc_id, const char* name, const H5L_info_t*,
                       void* parentpointer)
{
	HDF5Object obj;
	obj.name   = std::string(name);
	obj.parent = static_cast<HDF5Object*>(parentpointer);

	H5O_info_t infobuf;
	H5Oget_info_by_name(loc_id, name, &infobuf, H5P_DEFAULT);
	obj.type = infobuf.type;

	switch(obj.type)
	{
		case H5O_TYPE_GROUP:
			obj.id = H5Gopen1(loc_id, name);
			H5Literate_by_name(loc_id, name, H5_INDEX_NAME, H5_ITER_NATIVE,
			                   NULL, iterateCallback, static_cast<void*>(&obj),
			                   H5P_DEFAULT);
			obj.parent->links.push_back(obj);
			break;
		case H5O_TYPE_DATASET:
			obj.id = H5Dopen1(loc_id, name);
			obj.parent->links.push_back(obj);
			break;
		case H5O_TYPE_NAMED_DATATYPE:
		case H5O_TYPE_NTYPES:
		case H5O_TYPE_UNKNOWN:
			break;
	}

	return 0;
}

HDF5Object readHDF5RootObject(std::string const& filePath)
{
	HDF5Object root;
	root.name   = "/";
	root.parent = nullptr;
	root.id     = H5Fopen(filePath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);

	H5O_info_t infobuf;
	H5Oget_info_by_name(root.id, ".", &infobuf, H5P_DEFAULT);
	root.type = infobuf.type;

	H5Literate(root.id, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, iterateCallback,
	           static_cast<void*>(&root));

	H5Fclose(root.id);
	return root;
}
