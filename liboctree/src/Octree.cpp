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

std::string Octree::tabs    = "";
std::ofstream Octree::debug = std::ofstream("LIBOCTREE.debug");

unsigned int Octree::totalNumberOfVertices = 0;

void Octree::init(std::vector<float>& data)
{
	totalNumberOfVertices = (data.size() / 3) - 1;
	std::cout.precision(3);
	init(data, 0, (data.size() / 3) - 1);
	std::cout.precision(6);
}

void Octree::init(std::vector<float>& data, unsigned int beg, unsigned int end)
{
	unsigned int verticesNumber(end - beg + 1);
	totalDataSize = 3 * verticesNumber;
	for(unsigned int i(beg); i <= end; ++i)
	{
		if(get(data, i, 0) < minX)
			minX = get(data, i, 0);
		if(get(data, i, 0) > maxX)
			maxX = get(data, i, 0);
		if(get(data, i, 1) < minY)
			minY = get(data, i, 1);
		if(get(data, i, 1) > maxY)
			maxY = get(data, i, 1);
		if(get(data, i, 2) < minZ)
			minZ = get(data, i, 2);
		if(get(data, i, 2) > maxZ)
			maxZ = get(data, i, 2);

		if(verticesNumber <= MAX_LEAF_SIZE
		   || (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		          < MAX_LEAF_SIZE / (float) verticesNumber)
		{
			this->data.push_back(get(data, i, 0));
			this->data.push_back(get(data, i, 1));
			this->data.push_back(get(data, i, 2));
		}
	}
	if(verticesNumber <= MAX_LEAF_SIZE)
	{
		// delete our part of the vector, we know we are at the end of the
		// vector per (*) (check after all the orderPivot calls)
		data.resize(3 * beg);
		showProgress(1.f - beg/(float)totalNumberOfVertices);
		// we don't need to create children
		return;
	}

	float midX((minX + maxX) / 2.f), midY((minY + maxY) / 2.f),
	    midZ((minZ + maxZ) / 2.f);

	// To construct the subtrees we will swap elements within the vector and
	// split it at 7 places to have 8 parts, each corresponding to a subtree.
	// The splitting is done with this priority : x, then y, then z. For each
	// coordinate i, midI will be used as a pivot to put each point whose i
	// coordinate is below midI before the i split, and each point whose i
	// coordinate is above midI after the i split. When a split is done, it is
	// equivalent to having two vectors instead of one, but for memory sake we
	// keep everything in one single big vector with "splits" barriers.
	//
	// After this part of the algorithm, the data should be structure like this
	// :
	//
	// child0 - splits[0] - child1 - splits[1] - child2 - splits[2] - child3 -
	//             z                    y                    z
	//
	// splits[3] - child4 - splits[4] - child5 - splits[5] - child6 - splits[6]
	//    x                    z                    y                    z
	//
	// - child7

	unsigned int splits[7];

	// split along x in half
	splits[3] = orderPivot(data, beg, end, 0, midX);

	// split each half along y in quarters
	splits[1] = orderPivot(data, beg, splits[3] - 1, 1, midY);
	splits[5] = orderPivot(data, splits[3], end, 1, midY);

	// split each quarter by z in eighths
	splits[0] = orderPivot(data, beg, splits[1] - 1, 2, midZ);
	splits[2] = orderPivot(data, splits[1], splits[3] - 1, 2, midZ);
	splits[4] = orderPivot(data, splits[3], splits[5] - 1, 2, midZ);
	splits[6] = orderPivot(data, splits[5], end, 2, midZ);

	// Now we just assign each child its part
	// (*) we do it from end to begin to let the child use resize to free its
	// part of the vector (and not erase, which is less efficient)

	if(end > splits[6])
	{
		children[0] = newOctree();
		children[0]->init(data, splits[6], end);
	}
	for(unsigned int i(6); i > 0; --i)
	{
		if(splits[i] > splits[i - 1])
		{
			children[7 - i] = newOctree();
			children[7 - i]->init(data, splits[i - 1], splits[i] - 1);
		}
	}
	if(splits[0] > beg)
	{
		children[7] = newOctree();
		children[7]->init(data, beg, splits[0] - 1);
	}
}

void Octree::init(std::istream& in)
{
	brw::read(in, file_addr);
	debug << tabs << file_addr << std::endl;

	totalDataSize = 0;
	int64_t readVal;
	unsigned int i(0);
	while(true)
	{
		brw::read(in, readVal);
		debug << tabs << readVal << std::endl;
		if(readVal == 1) //) <= own end
			break;

		if(readVal == 0) //( <= sub node
		{
			tabs += '\t';
			children[i] = newOctree();
			children[i]->init(in);
			tabs.pop_back();
			totalDataSize += children[i]->totalDataSize;
		}
		else if(readVal != -1) // null node
		{
			tabs += '\t';
			children[i] = newOctree();
			children[i]->init(readVal, in);
			tabs.pop_back();
			totalDataSize += children[i]->totalDataSize;
		}
		++i;
	}
}

void Octree::init(int64_t file_addr, std::istream& in)
{
	int64_t cursor(in.tellg());

	this->file_addr = file_addr;
	in.seekg(file_addr + 6 * sizeof(float));
	uint32_t size;
	brw::read(in, size);
	totalDataSize = size;

	in.seekg(cursor);
}

bool Octree::isLeaf() const
{
	for(unsigned int i(0); i < 8; ++i)
		if(children[i] != nullptr)
			return false;
	return true;
}

std::vector<int64_t> Octree::getCompactData() const
{
	if(isLeaf())
		return std::vector<int64_t>({file_addr});

	std::vector<int64_t> res;
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
			for(int64_t d : children[i]->getCompactData())
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
	readOwnData(in);
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i] != nullptr)
			children[i]->readData(in);
	}
}

void Octree::readOwnData(std::istream& in)
{
	readBBox(in);
	brw::read(in, data);
}

void Octree::readBBoxes(std::istream& in)
{
	readBBox(in);
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i] != nullptr)
			children[i]->readBBoxes(in);
	}
}

void Octree::readBBox(std::istream& in)
{
	in.seekg(file_addr);
	brw::read(in, minX);
	brw::read(in, maxX);
	brw::read(in, minY);
	brw::read(in, maxY);
	brw::read(in, minZ);
	brw::read(in, maxZ);
}

std::vector<float> Octree::getAllData() const
{
	std::vector<float> result(data);
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i])
		{
			std::vector<float> childResult(children[i]->getAllData());
			result.insert(result.end(), childResult.begin(), childResult.end());
		}
	}
	return result;
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

Octree* Octree::newOctree() const
{
	return new Octree();
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
	// if root is a leaf, surround it with parenthesis
	if(headerSize == 1)
		headerSize += 2;

	int64_t start(stream.tellp());

	// write zeros to leave space, don't write first '('
	int64_t zero(0);
	for(unsigned int i(1); i < headerSize; ++i)
		brw::write(stream, zero);

	// write chunks and hold their addresses
	octree.writeData(stream);

	// write real compact data (with true addresses)
	stream.seekp(start);
	// we don't want to write the vector's size and the first '('
	// so we write manually from the second element
	std::vector<int64_t> header(octree.getCompactData());
	if(header.size() == 1)
	{
		brw::write(stream, header[0]);
		int64_t closingParenthesis(1);
		brw::write(stream, closingParenthesis);
	}
	else
	{
		std::ofstream debug("LIBOCTREE.debug");
		std::string tabs = "";
		for(unsigned int i(0); i < header.size(); ++i)
		{
			if(header[i] == 1)
				tabs.pop_back();
			debug << tabs << header[i] << std::endl;
			if(header[i] == 0)
				tabs += '\t';
		}
		brw::write(stream, header[1], headerSize - 1);
	}
}

inline float Octree::get(std::vector<float> const& data, unsigned int vertex,
                  unsigned int dim)
{
	return data[3 * vertex + dim];
}

inline void Octree::set(std::vector<float>& data, unsigned int vertex,
                 unsigned int dim, float val)
{
	data[3 * vertex + dim] = val;
}

void Octree::swap(std::vector<float>& data, unsigned int i, unsigned int j)
{
	float x(get(data, i, 0)), y(get(data, i, 1)), z(get(data, i, 2));

	set(data, i, 0, get(data, j, 0));
	set(data, i, 1, get(data, j, 1));
	set(data, i, 2, get(data, j, 2));
	set(data, j, 0, x);
	set(data, j, 1, y);
	set(data, j, 2, z);
}

unsigned int Octree::orderPivot(std::vector<float>& data, unsigned int beg,
                                unsigned int end, unsigned int dim, float pivot)
{
	// if end == beg - 1, this is not too bad, so let it go
	if(beg != 0 && end < (beg-1))
	{
		std::cout << "\r\nCONSISTENCY ERROR : " << beg << " " << end << std::endl;
		exit(1);
	}
	if(beg == end)
		return beg;

	unsigned int begBAK(beg), endBAK(end);
	while(beg < end)
	{
		if(get(data, beg, dim) >= pivot && get(data, end, dim) < pivot)
			swap(data, beg, end);
		if(get(data, beg, dim) < pivot)
			++beg;
		if(get(data, end, dim) >= pivot)
			--end;
	}
	unsigned int split = beg;
	while(get(data, split, dim) > pivot && beg != begBAK)
		--split;
	while(get(data, split, dim) < pivot && end != endBAK)
		++split;

	return split;
}

void Octree::showProgress(float progress)
{
	const unsigned int barSize(78);
	unsigned int numberOfXs(progress * barSize);
	std::cout << "\r\033[K" << '[';
	for(unsigned int i(0); i < numberOfXs; ++i)
		std::cout << "\u25B1";
	for(unsigned int i(0); i < barSize-numberOfXs; ++i)
		std::cout << ' ';
	std::cout << "] " << static_cast<unsigned int>(100 * progress) << '%';
	std::fflush(stdout);
	if(progress == 1.f)
		std::cout << std::endl;
}
