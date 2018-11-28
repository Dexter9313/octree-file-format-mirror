#include "Octree.hpp"

Octree::Octree(std::vector<float> data)
    : Leaf(data)
{
	std::vector<float>* subvectors[8];
	for(unsigned int i(0); i < 8; ++i)
		subvectors[i] = new std::vector<float>;
	float midX((minX + maxX) / 2.0), midY((minY + maxY) / 2.0),
	    midZ((minZ + maxZ) / 2.0);
	for(unsigned int i(0); i < data.size(); i += 3)
	{
		unsigned int addr = 0;
		if(data[i] > midX)
			addr += 4;
		if(data[i + 1] > midY)
			addr += 2;
		if(data[i + 2] > midZ)
			addr += 1;
		subvectors[addr]->push_back(data[i]);
		subvectors[addr]->push_back(data[i + 1]);
		subvectors[addr]->push_back(data[i + 2]);
	}

	for(unsigned int i(0); i < 8; ++i)
	{
		if(subvectors[i]->size() > 3 * MAX_LEAF_SIZE)
		{
			children[i] = new Octree(*subvectors[i]);
		}
		else if(subvectors[i]->size() > 0)
		{
			children[i] = new Leaf(*subvectors[i]);
		}
		delete subvectors[i];
	}
}

Octree::Octree(std::ifstream& in)
    : Leaf(in)
{
	long readVal;
	unsigned int i(0);
	while(true)
	{
		std::streampos pos(in.tellg());
		brw::read(in, readVal);
		if(readVal == 1) //) <= own end
			break;

		if(readVal == 0) //( <= sub node
		{
			children[i] = new Octree(in);
		}
		else if(readVal != -1) // null node
		{
			in.seekg(pos);
			children[i] = new Leaf(in);
		}
		++i;
	}
}

std::vector<long> Octree::getCompactData() const
{
	std::vector<long> res;
	res.push_back(0); // (
	res.push_back(file_addr);
	// compute nb of children to write (if last ones are nullptr, no need to
	// write them)
	unsigned int nb(8);
	while(nb > 0 && children[nb - 1] == nullptr)
		nb--;
	// append compact data of children
	for(unsigned int i(0); i < nb; ++i)
	{
		if(children[i] != nullptr)
		{
			for(long d : children[i]->getCompactData())
				res.push_back(d);
		}
		else
		{
			res.push_back(-1); // null node
		}
	}
	res.push_back(1); // )
	return res;
}

void Octree::writeData(std::ofstream& out)
{
	Leaf::writeData(out);
	for(unsigned int i(0); i < 8; ++i)
		if(children[i] != nullptr)
			children[i]->writeData(out);
}

void Octree::readData(std::ifstream& in)
{
	Leaf::readData(in);
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i] != nullptr)
			children[i]->readData(in);
	}
}

void Octree::debug(std::string const& tabs) const
{
	std::cout << tabs << "D:" << std::endl;
	Leaf::debug(tabs);
	for(unsigned int i(0); i < 8; ++i)
	{
		std::cout << tabs << i << ":" << std::endl;
		if(children[i] != nullptr)
			children[i]->debug(tabs + "\t");
	}
}

Octree::~Octree()
{
	for(Leaf* t : children)
	{
		if(t != nullptr)
			delete t;
	}
}

void write(std::ofstream& stream, Octree& octree)
{
	uint32_t headerSize(octree.getCompactData().size());
	long start(stream.tellp());
	long zero;
	for(unsigned int i(0); i < headerSize; ++i)
		brw::write(stream, zero);
	octree.writeData(stream);

	stream.seekp(start);
	std::vector<long> headerData(octree.getCompactData());
	brw::write(stream, headerData);
}
