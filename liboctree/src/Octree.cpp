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

// std::string Octree::tabs    = "";
// std::ofstream Octree::debug = std::ofstream("LIBOCTREE.debug");

unsigned int Octree::threadsLaunched    = 0;
std::mutex Octree::threadsLaunchedMutex = {};
size_t Octree::verticesLoaded;
std::mutex Octree::verticesLoadedMutex = {};

size_t Octree::totalNumberOfVertices = 0;

Octree::Octree()
    : rootManagedCommonData(new CommonData)
    , commonData(*rootManagedCommonData)
{
}

Octree::Octree(CommonData& commonData)
    : commonData(commonData)
{
}

void Octree::setFlags(Flags flags)
{
	commonData.flags        = flags;
	commonData.dimPerVertex = 3;
	if((flags & Flags::STORE_RADIUS) != Flags::NONE)
		++commonData.dimPerVertex;
	if((flags & Flags::STORE_LUMINOSITY) != Flags::NONE)
		++commonData.dimPerVertex;
	if((flags & Flags::STORE_COLOR) != Flags::NONE)
		commonData.dimPerVertex += 3;
	if((flags & Flags::STORE_DENSITY) != Flags::NONE)
		++commonData.dimPerVertex;
	if((flags & Flags::STORE_TEMPERATURE) != Flags::NONE)
		++commonData.dimPerVertex;
}

void Octree::init(std::vector<float>& data)
{
	totalNumberOfVertices = (data.size() / commonData.dimPerVertex) - 1;
	std::cout.precision(3);
	initParallel(&data, 0, (data.size() / commonData.dimPerVertex) - 1);
	std::cout.precision(6);
}

void Octree::init(std::vector<float>& data, size_t beg, size_t end)
{
	size_t verticesNumber(end - beg + 1);
	if(verticesNumber <= MAX_LEAF_SIZE)
	{
		this->data.setAsReference(&data, beg * commonData.dimPerVertex,
		                          (end + 1) * commonData.dimPerVertex - 1);
	}
	else
	{
		this->data.setAsVector();
		this->data.asVector().reserve(MAX_LEAF_SIZE * commonData.dimPerVertex);
	}
	totalDataSize = commonData.dimPerVertex * verticesNumber;
	for(size_t i(beg); i <= end; ++i)
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

		if(verticesNumber > MAX_LEAF_SIZE
		   && this->data.size() < MAX_LEAF_SIZE * commonData.dimPerVertex
		   && (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		          < MAX_LEAF_SIZE / (float) verticesNumber)
		{
			for(unsigned int j(0); j < commonData.dimPerVertex; ++j)
			{
				this->data.push_back(get(data, i, j));
			}
		}
	}
	if((commonData.flags & Flags::NORMALIZED_NODES) != Flags::NONE)
	{
		float localScale(1.f);
		if((maxX - minX > maxY - minY) && (maxX - minX > maxZ - minZ))
		{
			localScale = maxX - minX;
		}
		else if(maxY - minY > maxZ - minZ)
		{
			localScale = maxY - minY;
		}
		else if(maxZ != minZ)
		{
			localScale = maxZ - minZ;
		}

		for(size_t i(0); i < this->data.size(); i += commonData.dimPerVertex)
		{
			this->data[i] -= minX;
			this->data[i] /= localScale;
			this->data[i + 1] -= minY;
			this->data[i + 1] /= localScale;
			this->data[i + 2] -= minZ;
			this->data[i + 2] /= localScale;
		}
	}
	if(verticesNumber <= MAX_LEAF_SIZE)
	{
		showProgress(1.f - beg / (float) totalNumberOfVertices);
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

	size_t splits[7];

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

	if(splits[0] < beg || splits[6] > end)
	{
		std::cout << "ERR0" << std::endl;
		int* foo = (int*) 0x10;
		*foo     = 0;
	}
	for(unsigned int i(0); i < 6; ++i)
	{
		if(splits[i] > splits[i + 1])
		{
			std::cout << "ERR1 " << i << std::endl;
			int* foo = (int*) 0x10;
			*foo     = 0;
		}
	}

	// Now we just assign each child its part
	// (*) we do it from end to begin to let the child use resize to free its
	// part of the vector (and not erase, which is less efficient)

	if(end > splits[6])
	{
		children[0] = newChild();
		children[0]->init(data, splits[6], end);
	}
	for(unsigned int i(6); i > 0; --i)
	{
		if(splits[i] > splits[i - 1])
		{
			children[7 - i] = newChild();
			children[7 - i]->init(data, splits[i - 1], splits[i] - 1);
		}
	}
	if(splits[0] > beg)
	{
		children[7] = newChild();
		children[7]->init(data, beg, splits[0] - 1);
	}
}

#include <thread>

void Octree::initParallel(std::vector<float>* data, size_t beg, size_t end)
{
	size_t verticesNumber(end - beg + 1);
	if(verticesNumber <= MAX_LEAF_SIZE)
	{
		this->data.setAsReference(data, beg * commonData.dimPerVertex,
		                          (end + 1) * commonData.dimPerVertex - 1);
	}
	else
	{
		this->data.setAsVector();
		this->data.asVector().reserve(MAX_LEAF_SIZE * commonData.dimPerVertex);
	}
	totalDataSize = commonData.dimPerVertex * verticesNumber;
	for(size_t i(beg); i <= end; ++i)
	{
		if(get(*data, i, 0) < minX)
			minX = get(*data, i, 0);
		if(get(*data, i, 0) > maxX)
			maxX = get(*data, i, 0);
		if(get(*data, i, 1) < minY)
			minY = get(*data, i, 1);
		if(get(*data, i, 1) > maxY)
			maxY = get(*data, i, 1);
		if(get(*data, i, 2) < minZ)
			minZ = get(*data, i, 2);
		if(get(*data, i, 2) > maxZ)
			maxZ = get(*data, i, 2);

		if(verticesNumber > MAX_LEAF_SIZE
		   && this->data.size() < MAX_LEAF_SIZE * commonData.dimPerVertex
		   && (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		          < MAX_LEAF_SIZE / (float) verticesNumber)
		{
			for(unsigned int j(0); j < commonData.dimPerVertex; ++j)
			{
				this->data.push_back(get(*data, i, j));
			}
		}
	}
	if((commonData.flags & Flags::NORMALIZED_NODES) != Flags::NONE)
	{
		float localScale(1.f);
		if((maxX - minX > maxY - minY) && (maxX - minX > maxZ - minZ))
		{
			localScale = maxX - minX;
		}
		else if(maxY - minY > maxZ - minZ)
		{
			localScale = maxY - minY;
		}
		else if(maxZ != minZ)
		{
			localScale = maxZ - minZ;
		}

		for(size_t i(0); i < this->data.size(); i += commonData.dimPerVertex)
		{
			this->data[i] -= minX;
			this->data[i] /= localScale;
			this->data[i + 1] -= minY;
			this->data[i + 1] /= localScale;
			this->data[i + 2] -= minZ;
			this->data[i + 2] /= localScale;
		}
	}
	if(verticesNumber <= MAX_LEAF_SIZE)
	{
		// delete our part of the vector, we know we are at the end of the
		// vector per (*) (check after all the orderPivot calls)
		// data.resize(commonData.dimPerVertex * beg);
		std::lock_guard<std::mutex> guard(verticesLoadedMutex);
		verticesLoaded += verticesNumber;
		showProgress(verticesLoaded / (float) totalNumberOfVertices);
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

	size_t splits[7];

	// split along x in half
	splits[3] = orderPivot(*data, beg, end, 0, midX);

	// split each half along y in quarters
	splits[1] = orderPivot(*data, beg, splits[3] - 1, 1, midY);
	splits[5] = orderPivot(*data, splits[3], end, 1, midY);

	// split each quarter by z in eighths
	splits[0] = orderPivot(*data, beg, splits[1] - 1, 2, midZ);
	splits[2] = orderPivot(*data, splits[1], splits[3] - 1, 2, midZ);
	splits[4] = orderPivot(*data, splits[3], splits[5] - 1, 2, midZ);
	splits[6] = orderPivot(*data, splits[5], end, 2, midZ);

	if(splits[0] < beg || splits[6] > end)
	{
		std::cout << "ERR0" << std::endl;
		int* foo = (int*) 0x10;
		*foo     = 0;
	}
	for(unsigned int i(0); i < 6; ++i)
	{
		if(splits[i] > splits[i + 1])
		{
			std::cout << "ERR1 " << i << std::endl;
			int* foo = (int*) 0x10;
			*foo     = 0;
		}
	}

	// Now we just assign each child its part
	// (*) we do it from end to begin to let the child use resize to free its
	// part of the vector (and not erase, which is less efficient)

	if(threadsLaunched + 8 > std::thread::hardware_concurrency())
	{
		if(end > splits[6])
		{
			children[0] = newChild();
			children[0]->init(*data, splits[6], end);
		}
		for(unsigned int i(6); i > 0; --i)
		{
			if(splits[i] > splits[i - 1])
			{
				children[7 - i] = newChild();
				children[7 - i]->init(*data, splits[i - 1], splits[i] - 1);
			}
		}
		if(splits[0] > beg)
		{
			children[7] = newChild();
			children[7]->init(*data, beg, splits[0] - 1);
		}
	}
	else
	{
		std::thread* threads[8] = {nullptr, nullptr, nullptr, nullptr,
		                           nullptr, nullptr, nullptr, nullptr};

		if(end > splits[6])
		{
			children[0] = newChild();
			threads[0]  = new std::thread(&Octree::initParallel, children[0],
			                              data, splits[6], end);
		}
		for(unsigned int i(6); i > 0; --i)
		{
			if(splits[i] > splits[i - 1])
			{
				children[7 - i] = newChild();
				threads[7 - i]
				    = new std::thread(&Octree::initParallel, children[7 - i],
				                      data, splits[i - 1], splits[i] - 1);
			}
		}
		if(splits[0] > beg)
		{
			children[7] = newChild();
			threads[7]  = new std::thread(&Octree::initParallel, children[7],
			                              data, beg, splits[0] - 1);
		}

		for(unsigned int i(0); i < 8; ++i)
		{
			if(threads[i] != nullptr)
			{
				threads[i]->join();
				delete threads[i];
			}
		}
	}
}

void Octree::init(std::istream& in)
{
	this->data.setAsVector();
	brw::read(in, file_addr);
	// debug << tabs << file_addr << std::endl;

	// if file_addr is negative, we are actually at the beginning of the file
	// and we have flags to read !
	if(file_addr < 0)
	{
		commonData.versionMajor = 0;
		commonData.versionMinor = 0;
		Flags flags_temp;
		brw::read(in, flags_temp);
		setFlags(flags_temp);
		// if versioned
		if((commonData.flags & Flags::VERSIONED) != Flags::NONE)
		{
			brw::read(in, commonData.versionMajor);
			brw::read(in, commonData.versionMinor);
			if(commonData.versionMajor < VERSION_MAJOR
			   || (commonData.versionMajor == VERSION_MAJOR
			       && (int32_t) commonData.versionMinor < VERSION_MINOR))
			{
				std::cerr << "Error: this version of liboctree can read octree "
				             "files up to format version "
				          << VERSION_MAJOR << "." << VERSION_MINOR
				          << " whereas you are trying to load a file encoded "
				             "with version "
				          << commonData.versionMajor << "."
				          << commonData.versionMinor
				          << ". Upgrade liboctree to do so." << std::endl;
				exit(EXIT_FAILURE);
			}
		}
		std::cout << "Reading octree file version " << commonData.versionMajor
		          << "." << commonData.versionMinor << std::endl;
		// read file_addr again with the real value this time
		brw::read(in, file_addr);
	}
	init(file_addr, in);

	totalDataSize = 0;
	int64_t readVal;
	unsigned int i(0);
	while(true)
	{
		brw::read(in, readVal);
		// debug << tabs << readVal << std::endl;
		if(readVal == 1) //) <= own end
			break;

		if(readVal == 0) //( <= sub node
		{
			// tabs += '\t';
			children[i] = newChild();
			children[i]->init(in);
			// tabs.pop_back();
			totalDataSize += children[i]->totalDataSize;
		}
		else if(readVal != -1) // null node
		{
			// tabs += '\t';
			children[i] = newChild();
			children[i]->init(readVal, in);
			// tabs.pop_back();
			totalDataSize += children[i]->totalDataSize;
		}
		++i;
	}
}

void Octree::init(int64_t file_addr, std::istream& in)
{
	this->data.setAsVector();
	if(commonData.versionMajor < 2)
	{
		init1_0(file_addr, in);
	}
	else
	{
		init2_0(file_addr, in);
	}
}

void Octree::init1_0(int64_t file_addr, std::istream& in)
{
	int64_t cursor(in.tellg());

	this->file_addr = file_addr;
	in.seekg(file_addr + 6 * sizeof(float));
	uint64_t size;
	brw::read(in, size);
	totalDataSize = size;

	in.seekg(cursor);
}

void Octree::init2_0(int64_t file_addr, std::istream& in)
{
	this->file_addr = file_addr;
	uint64_t totalDataSizeUINT64;
	brw::read(in, totalDataSizeUINT64);
	totalDataSize = totalDataSizeUINT64;
	brw::read(in, minX);
	brw::read(in, maxX);
	brw::read(in, minY);
	brw::read(in, maxY);
	brw::read(in, minZ);
	brw::read(in, maxZ);
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
	std::vector<int64_t> res;
	if(!isLeaf())
	{
		res.push_back(0); // (
	}
	res.push_back(file_addr);
	res.push_back(totalDataSize);
	std::array<uint64_t, 3> bboxUint64(getBoundingBoxUint64Representation());
	res.push_back(bboxUint64[0]);
	res.push_back(bboxUint64[1]);
	res.push_back(bboxUint64[2]);
	if(isLeaf())
	{
		return res;
	}

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
	uint64_t size(data.size());
	brw::write(out, size);
	brw::write(out, data[0], data.size());

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
	if(commonData.versionMajor < 2)
	{
		readOwnData1_0(in);
	}
	else
	{
		readOwnData2_0(in);
	}
}

void Octree::readOwnData1_0(std::istream& in)
{
	readBBox(in);
	uint64_t size;
	brw::read(in, size);
	data.asVector().resize(size);
	brw::read(in, data[0], data.size());
}

void Octree::readOwnData2_0(std::istream& in)
{
	in.seekg(file_addr);
	uint64_t size;
	brw::read(in, size);
	data.asVector().resize(size);
	brw::read(in, data[0], data.size());
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
	if(commonData.versionMajor < 2)
	{
		readBBox1_0(in);
	}
	else
	{
		readBBox2_0(in);
	}
}

void Octree::readBBox1_0(std::istream& in)
{
	in.seekg(file_addr);
	brw::read(in, minX);
	brw::read(in, maxX);
	brw::read(in, minY);
	brw::read(in, maxY);
	brw::read(in, minZ);
	brw::read(in, maxZ);
}

void Octree::readBBox2_0(std::istream& /*in*/) {}

std::vector<float> Octree::getOwnData() const
{
	std::vector<float> result(data.asVector());
	if((commonData.flags & Flags::NORMALIZED_NODES) != Flags::NONE)
	{
		float localScale(1.f);
		if((maxX - minX > maxY - minY) && (maxX - minX > maxZ - minZ))
		{
			localScale = maxX - minX;
		}
		else if(maxY - minY > maxZ - minZ)
		{
			localScale = maxY - minY;
		}
		else if(maxZ != minZ)
		{
			localScale = maxZ - minZ;
		}

		for(size_t i(0); i < this->data.size(); i += commonData.dimPerVertex)
		{
			result[i] *= localScale;
			result[i] += minX;
			result[i + 1] *= localScale;
			result[i + 1] += minY;
			result[i + 2] *= localScale;
			result[i + 2] += minZ;
		}
	}
	return result;
}

std::vector<float> Octree::getData() const
{
	if(isLeaf())
	{
		return getOwnData();
	}

	std::vector<float> result;
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i])
		{
			std::vector<float> childResult(children[i]->getData());
			result.insert(result.end(), childResult.begin(), childResult.end());
		}
	}
	return result;
}

void Octree::dumpInVectorAndEmpty(std::vector<float>& vector)
{
	if(isLeaf())
	{
		auto d(getOwnData());
		vector.insert(vector.end(), d.begin(), d.end());
		data.asVector().resize(0);
		data.asVector().shrink_to_fit();
		return;
	}
	for(unsigned int i(0); i < 8; ++i)
	{
		if(children[i])
		{
			children[i]->dumpInVectorAndEmpty(vector);
		}
	}
}

std::string Octree::toString(std::string const& tabs) const
{
	std::ostringstream oss;
	oss << tabs << "D:" << std::endl;
	oss << tabs << "BBox:" << minX << "->" << maxX << ";" << minY << "->"
	    << maxY << ";" << minZ << ";" << maxZ << std::endl;
	for(size_t i(0); i < data.size(); i += commonData.dimPerVertex)
	{
		for(unsigned int j(0); j < commonData.dimPerVertex; ++j)
		{
			oss << tabs << data[i + j] << "; ";
		}
		oss << std::endl;
	}
	for(unsigned int i(0); i < 8; ++i)
	{
		oss << tabs << i << ":" << std::endl;
		if(children[i] != nullptr)
			oss << children[i]->toString(tabs + "\t");
	}
	return oss.str();
}

Octree* Octree::newChild() const
{
	return new Octree(commonData);
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
	int64_t start(stream.tellp());

	// write flags
	int64_t minusone(-1);
	// force versioned flag
	uint64_t flags(
	    static_cast<uint64_t>(octree.getFlags() | Octree::Flags::VERSIONED));
	brw::write(stream, minusone);
	brw::write(stream, flags);
	uint32_t versionMajor(VERSION_MAJOR);
	uint32_t versionMinor(VERSION_MINOR);
	brw::write(stream, versionMajor);
	brw::write(stream, versionMinor);

	// write the rest of the tree
	uint64_t headerSize(octree.getCompactData().size());
	// if root is a leaf, surround it with parenthesis
	if(headerSize == 5)
		headerSize += 2;

	int64_t headerStart(stream.tellp());

	// write zeros to leave space, don't write first '('
	int64_t zero(0);
	for(size_t i(1); i < headerSize; ++i)
		brw::write(stream, zero);

	int64_t negDataStart(-1 * stream.tellp());
	// write chunks and hold their addresses
	octree.writeData(stream);

	// replace the initial -1 by -1*dataStart
	stream.seekp(start);
	brw::write(stream, negDataStart);

	// write real compact data (with true addresses)
	stream.seekp(headerStart);
	// we don't want to write the vector's size and the first '('
	// so we write manually from the second element
	std::vector<int64_t> header(octree.getCompactData());
	if(header.size() == 5)
	{
		// write(stream, header) would write header size
		brw::write(stream, header[0]);
		brw::write(stream, header[1]);
		brw::write(stream, header[2]);
		brw::write(stream, header[3]);
		brw::write(stream, header[4]);
		int64_t closingParenthesis(1);
		brw::write(stream, closingParenthesis);
	}
	else
	{
		/*std::ofstream debug("LIBOCTREE.debug");
		std::string tabs = "";
		for(size_t i(0); i < header.size(); ++i)
		{
		    if(header[i] == 1)
		        tabs.pop_back();
		    debug << tabs << header[i] << std::endl;
		    if(header[i] == 0)
		        tabs += '\t';
		}*/
		brw::write(stream, header[1], headerSize - 1);
	}
}

Octree::Flags operator~(Octree::Flags f)
{
	return static_cast<Octree::Flags>(~static_cast<uint64_t>(f));
}

Octree::Flags operator|(Octree::Flags a, Octree::Flags b)
{
	return static_cast<Octree::Flags>(static_cast<uint64_t>(a)
	                                  | static_cast<uint64_t>(b));
}

Octree::Flags& operator|=(Octree::Flags& a, Octree::Flags b)
{
	a = a | b;
	return a;
}

Octree::Flags operator&(Octree::Flags a, Octree::Flags b)
{
	return static_cast<Octree::Flags>(static_cast<uint64_t>(a)
	                                  & static_cast<uint64_t>(b));
}

Octree::Flags& operator&=(Octree::Flags& a, Octree::Flags b)
{
	a = a & b;
	return a;
}

std::array<uint64_t, 3> Octree::getBoundingBoxUint64Representation() const
{
	std::array<float, 6> bboxFloat{{minX, maxX, minY, maxY, minZ, maxZ}};
	uint64_t* data(reinterpret_cast<uint64_t*>(&(bboxFloat[0])));
	std::array<uint64_t, 3> result{{data[0], data[1], data[2]}};
	return result;
}

inline float Octree::get(std::vector<float> const& data, size_t vertex,
                         unsigned int dim)
{
	return data[commonData.dimPerVertex * vertex + dim];
}

inline void Octree::set(std::vector<float>& data, size_t vertex,
                        unsigned int dim, float val)
{
	data[commonData.dimPerVertex * vertex + dim] = val;
}

void Octree::swap(std::vector<float>& data, size_t i, size_t j)
{
	std::vector<float> v(commonData.dimPerVertex);

	for(unsigned int k(0); k < commonData.dimPerVertex; ++k)
	{
		float tmp = get(data, i, k);
		set(data, i, k, get(data, j, k));
		set(data, j, k, tmp);
	}
}

size_t Octree::orderPivot(std::vector<float>& data, size_t beg, size_t end,
                          unsigned int dim, float pivot)
{
	// check if -1 has been passed as end (split[i]-1 if split[i] == 0)
	size_t max(0);
	--max;
	if(end == max)
	{
		return beg;
	}

	// if end == beg - 1, this is not too bad, so let it go
	if(beg != 0 && end < (beg - 1))
	{
		std::cout << "\r\nCONSISTENCY ERROR : " << beg << " " << end
		          << std::endl;
		exit(1);
	}
	if(beg == end)
		return beg;

	size_t begBAK(beg), endBAK(end);
	while(beg < end)
	{
		if(get(data, beg, dim) >= pivot && get(data, end, dim) < pivot)
			swap(data, beg, end);
		if(get(data, beg, dim) < pivot)
			++beg;
		if(get(data, end, dim) >= pivot)
			--end;
	}
	size_t split = beg;
	while(get(data, split, dim) > pivot && beg != begBAK)
		--split;
	while(get(data, split, dim) < pivot && end != endBAK)
		++split;

	return split;
}

void Octree::showProgress(float progress)
{
	if(progress > 1.f || progress < 0.f)
		return;
	const unsigned int barSize(78);
	unsigned int numberOfXs(progress * barSize);
	std::cout << "\r\033[K" << '[';
	for(unsigned int i(0); i < numberOfXs; ++i)
		std::cout << "\u25B1";
	for(unsigned int i(0); i < barSize - numberOfXs; ++i)
		std::cout << ' ';
	std::cout << "] " << static_cast<unsigned int>(100 * progress) << '%';
	std::fflush(stdout);
	if(progress == 1.f)
		std::cout << std::endl;
}
