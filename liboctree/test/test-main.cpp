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

#include <fstream>
#include <iostream>
#include <vector>

#include "Octree.hpp"
#include "binaryrw.hpp"

namespace term
{
const std::string red("\033[31;31m");
const std::string green("\033[32;32m");
const std::string reset("\033[0;0m");
} // namespace term

class TestBinaryFile : public std::fstream
{
  public:
	TestBinaryFile()
	    : std::fstream()
	{
		system("touch TESTS_");
		open("TESTS_",
		     std::fstream::in | std::fstream::out | std::fstream::binary);
	};
	void resetCursor()
	{
		this->seekg(0);
		this->seekp(0);
	};
	~TestBinaryFile()
	{
		close();
		system("rm TESTS_");
	};
};

template <typename T>
void TEST_EQUAL(T result, T shouldbe, const char* testname,
                std::string const& additional = "")
{
	if(result != shouldbe)
	{
		std::cerr.precision(2 * sizeof(T));
		std::cerr << term::red << "[FAILURE] " << term::reset << testname
		          << "(result is : \"" << result << "|" << std::hex
		          << *reinterpret_cast<uint64_t*>(&result) << std::dec
		          << "\", expected : \"" << shouldbe << "|" << std::hex
		          << *reinterpret_cast<uint64_t*>(&shouldbe) << "\")"
		          << (additional != "" ? " Additional Information : " : "")
		          << additional << std::endl;
		exit(EXIT_FAILURE);
	}
}

unsigned char* randomByteArray(size_t n)
{
	unsigned char* result = new unsigned char[n];
	for(size_t i(0); i < n; ++i)
		result[i] = rand();
	return result;
}

template <typename T>
T randomVal()
{
	unsigned char* byteArray = randomByteArray(sizeof(T));
	T result                 = *reinterpret_cast<T*>(byteArray);
	delete[] byteArray;
	return result;
}

std::vector<float> generateVertices(size_t number, unsigned int seed,
                                    unsigned int dimPerVertex = 3)
{
	std::vector<float> vertices;
	vertices.reserve(dimPerVertex * number);

	srand(seed);

	for(size_t i(0); i < dimPerVertex * number; ++i)
	{
		vertices.push_back(
		    2 * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		    - 1);
	}

	return vertices;
}

int main(int, char*[])
{
	const unsigned int seed = 0;
	srand(seed);
	std::string success(term::green + "[SUCCESS] " + term::reset);
	std::string failure(term::red + "[FAILURE] " + term::reset);
	// TEST BINARY RW double
	{
		double pi(3.14159265358979323846);
		TestBinaryFile f;
		brw::write(f, pi);
		f.resetCursor();
		double pi2;
		brw::read(f, pi2);
		if(pi != pi2)
		{
			std::cerr.precision(16);
			std::cerr << failure << "R/W double value (\"" << pi2
			          << "\" retrieved, should be \"" << pi2 << "\""
			          << std::endl;
			return EXIT_FAILURE;
		}
		std::cout << success << "R/W double value " << std::endl;
	}
	// TEST BINARY RW array of doubles
	{
		const unsigned int N(1000);
		double* array = new double[N];
		for(unsigned int i(0); i < N; ++i)
			array[i] = randomVal<double>();
		TestBinaryFile f;
		f.resetCursor();
		brw::write(f, array[0], N);
		f.resetCursor();
		double* array2 = new double[N];
		brw::read(f, array2[0], N);
		for(unsigned int i(0); i < N; ++i)
		{
			TEST_EQUAL(array2[i], array[i], "R/W array of doubles");
		}
		std::cout << success << "R/W array of doubles" << std::endl;
		delete[] array;
		delete[] array2;
	}
	// TEST BINARY RW vector of doubles
	{
		const unsigned int N(1000);
		std::vector<double> vec;
		for(unsigned int i(0); i < N; ++i)
			vec.push_back(randomVal<double>());
		TestBinaryFile f;
		f.resetCursor();
		brw::write(f, vec);
		f.resetCursor();
		std::vector<double> vec2;
		brw::read(f, vec2);
		for(unsigned int i(0); i < N; ++i)
		{
			TEST_EQUAL(vec2[i], vec[i], "R/W vector of doubles");
		}
		std::cout << success << "R/W vector of doubles" << std::endl;
	}
	const size_t bigTreeSize(200000);
	// TEST BINARY RW random octree
	{
		Octree octree1;
		std::vector<float> v(generateVertices(bigTreeSize, seed));
		octree1.init(v);
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2;
		octree2.init(f);
		octree2.readData(f);
		TEST_EQUAL(octree2.toString(), octree1.toString(), "R/W random octree");
		std::cout << success << "R/W random octree" << std::endl;
	}
	// TEST BINARY RW octree consisting of one leaf
	{
		Octree octree1;
		std::vector<float> v(generateVertices(10, seed));
		octree1.init(v);
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2;
		octree2.init(f);
		octree2.readData(f);
		TEST_EQUAL(octree2.toString(), octree1.toString(), "R/W random octree");
		std::cout << success << "R/W random octree (only one leaf)"
		          << std::endl;
	}
	// TEST OCTREE totalDataSize from data
	{
		Octree octree;
		std::vector<float> v(generateVertices(bigTreeSize, seed));
		octree.init(v);
		TEST_EQUAL(octree.getTotalDataSize(), bigTreeSize * 3,
		           "OCTREE totalDataSize (from data)");
		std::cout << success << "OCTREE totalDataSize (from data)" << std::endl;
	}
	// TEST OCTREE totalDataSize from file
	{
		Octree octree1;
		std::vector<float> v(generateVertices(bigTreeSize, seed));
		octree1.init(v);
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2;
		octree2.init(f);
		TEST_EQUAL(octree2.getTotalDataSize(), bigTreeSize * 3,
		           "OCTREE totalDataSize (from file)");
		std::cout << success << "OCTREE totalDataSize (from file)" << std::endl;
	}
	// TEST OCTREE flags RW
	{
		Octree::Flags flags(Octree::Flags::NORMALIZED_NODES
		                    | Octree::Flags::STORE_RADIUS
		                    | Octree::Flags::STORE_LUMINOSITY);
		Octree octree1(flags);
		std::vector<float> v(generateVertices(10, seed, 5));
		octree1.init(v);
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2(Octree::Flags::NONE);
		octree2.init(f);
		// remove VERSIONED as its meaningless from the user
		TEST_EQUAL(static_cast<uint64_t>(octree2.getFlags()
		                                 & ~Octree::Flags::VERSIONED),
		           static_cast<uint64_t>(flags & ~Octree::Flags::VERSIONED),
		           "R/W octree flags");
		std::cout << success << "R/W octree flags" << std::endl;
	}
	// TEST BINARY RW random octree with normalized nodes
	{
		Octree octree1(Octree::Flags::NORMALIZED_NODES);
		std::vector<float> v(generateVertices(bigTreeSize, seed));
		octree1.init(v);
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2;
		octree2.init(f);
		octree2.readData(f);
		TEST_EQUAL(octree2.toString(), octree1.toString(),
		           "R/W random octree with normalized nodes");
		std::cout << success << "R/W random octree with normalized nodes"
		          << std::endl;
	}
	// TEST BINARY RW random octree with more than three components per vertex
	{
		Octree octree1(Octree::Flags::STORE_RADIUS
		               | Octree::Flags::STORE_LUMINOSITY);
		std::vector<float> v(generateVertices(bigTreeSize, seed, 5));
		octree1.init(v);
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2;
		octree2.init(f);
		octree2.readData(f);
		TEST_EQUAL(octree2.getTotalDataSize(), bigTreeSize * 5,
		           "R/W random octree with more than three components per "
		           "vertex [size]");
		TEST_EQUAL(octree2.toString(), octree1.toString(),
		           "R/W random octree with more than three components per "
		           "vertex [content]");
		std::cout
		    << success
		    << "R/W random octree with more than three components per vertex"
		    << std::endl;
	}

	return EXIT_SUCCESS;
}
