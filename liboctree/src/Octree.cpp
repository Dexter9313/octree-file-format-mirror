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

#include "Octree.hpp"

Octree::Octree(std::vector<float> data)
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
	if(data.size() <= 3 * MAX_LEAF_SIZE)
		return; // we don't need to create children

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
		if(subvectors[i]->size() > 0)
		{
			children[i] = newOctree(*subvectors[i]);
		}
		delete subvectors[i];
	}
}

Octree::Octree(std::istream& in)
{
	brw::read(in, file_addr);

	long readVal;
	unsigned int i(0);
	while(true)
	{
		brw::read(in, readVal);
		if(readVal == 1) //) <= own end
			break;

		if(readVal == 0) //( <= sub node
			children[i] = newOctree(in);
		else if(readVal != -1) // null node
			children[i] = newOctree(readVal);
		++i;
	}
}

Octree::Octree(long file_addr)
    : file_addr(file_addr)
{
}

bool Octree::isLeaf() const
{
	for(unsigned int i(0); i < 8; ++i)
		if(children[i] != nullptr)
			return false;
	return true;
}

std::vector<long> Octree::getCompactData() const
{
	if(isLeaf())
		return std::vector<long>({file_addr});

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

void Octree::writeData(std::ostream& out)
{
	file_addr = out.tellp();
	brw::write(out, minX);
	brw::write(out, maxX);
	brw::write(out, minY);
	brw::write(out, maxY);
	brw::write(out, minZ);
	brw::write(out, maxZ);
	brw::write(out, data);
	for(unsigned int i(0); i < 8; ++i)
		if(children[i] != nullptr)
			children[i]->writeData(out);
}

void Octree::readData(std::istream& in)
{
	in.seekg(file_addr);
	brw::read(in, minX);
	brw::read(in, maxX);
	brw::read(in, minY);
	brw::read(in, maxY);
	brw::read(in, minZ);
	brw::read(in, maxZ);
	brw::read(in, data);
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i] != nullptr)
			children[i]->readData(in);
	}
}

std::string Octree::toString(std::string const& tabs) const
{
	std::ostringstream oss;
	oss << tabs << "D:" << std::endl;
	for(unsigned int i(0); i < data.size(); i += 3)
		oss << tabs << data[i] << "; " << data[i + 1] << "; " << data[i + 2]
		    << std::endl;
	for(unsigned int i(0); i < 8; ++i)
	{
		oss << tabs << i << ":" << std::endl;
		if(children[i] != nullptr)
			oss << children[i]->toString(tabs + "\t");
	}
	return oss.str();
}

Octree* Octree::newOctree(std::vector<float> data) const
{
	return new Octree(data);
}

Octree* Octree::newOctree(std::istream& in) const
{
	return new Octree(in);
}

Octree* Octree::newOctree(long file_addr) const
{
	return new Octree(file_addr);
}

Octree::~Octree()
{
	for(Octree* t : children)
	{
		if(t != nullptr)
			delete t;
	}
}

void write(std::ostream& stream, Octree& octree)
{
	uint32_t headerSize(octree.getCompactData().size());
	long start(stream.tellp());

	// write zeros to leave space, don't write first '('
	long zero(0);
	for(unsigned int i(1); i < headerSize; ++i)
		brw::write(stream, zero);

	// write chunks and hold their addresses
	octree.writeData(stream);

	// write real compact data (with true addresses)
	stream.seekp(start);
	// we don't need to write the vector's size and the first '('
	std::vector<long> header(octree.getCompactData());
	long u;
	for(unsigned int i(1); i < headerSize; ++i)
	{
		u = header[i];
		brw::write(stream, u);
	}
}
