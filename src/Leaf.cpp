#include "Leaf.hpp"

Leaf::Leaf(std::vector<float> const& data)
{
	for(unsigned int i(0); i < data.size(); i += 3)
	{
		if(data[i] < minX)
			minX = data[i];
		if(data[i] > maxX)
			maxX = data[i];
		if(data[i + 1] < minY)
			minY = data[i + 1];
		if(data[i + 1] > maxY)
			maxY = data[i + 1];
		if(data[i + 2] < minZ)
			minZ = data[i + 2];
		if(data[i + 2] > maxZ)
			maxZ = data[i + 2];

		if(data.size() <= 3 * MAX_LEAF_SIZE
		   || (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		          < (3 * MAX_LEAF_SIZE) / (float) data.size())
		{
			this->data.push_back(data[i]);
			this->data.push_back(data[i + 1]);
			this->data.push_back(data[i + 2]);
		}
	}
}

Leaf::Leaf(std::ifstream& in)
{
	brw::read(in, file_addr);
}

void Leaf::writeData(std::ofstream& out)
{
	file_addr = out.tellp();
	brw::write(out, minX);
	brw::write(out, maxX);
	brw::write(out, minY);
	brw::write(out, maxY);
	brw::write(out, minZ);
	brw::write(out, maxZ);
	brw::write(out, data);
}

void Leaf::readData(std::ifstream& in)
{
	in.seekg(file_addr);
	brw::read(in, minX);
	brw::read(in, maxX);
	brw::read(in, minY);
	brw::read(in, maxY);
	brw::read(in, minZ);
	brw::read(in, maxZ);
	brw::read(in, data);
}

void Leaf::debug(std::string const& tabs) const
{
	for(unsigned int i(0); i < data.size(); i += 3)
		std::cout << tabs << data[i] << "; " << data[i + 1] << "; "
		          << data[i + 2] << std::endl;
}
