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
} // term

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

unsigned char* randomByteArray(unsigned int n)
{
	unsigned char* result = new unsigned char[n];
	for(unsigned int i(0); i < n; ++i)
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

std::vector<float> generateVertices(unsigned int number, unsigned int seed)
{
	std::vector<float> vertices;
	vertices.reserve(3 * number);

	srand(seed);

	for(unsigned int i(0); i < 3 * number; ++i)
	{
		vertices.push_back(
		    2 * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		    - 1);
	}

	return vertices;
}

int main(int, char* [])
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
	// TEST BINARY RW random octree
	{
		Octree octree1;
		octree1.init(generateVertices(100000, seed));
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		Octree octree2;
		octree2.init(f);
		octree2.readData(f);
		TEST_EQUAL(octree1.toString(), octree2.toString(), "R/W random octree");
		std::cout << success << "R/W random octree" << std::endl;
	}

	return EXIT_SUCCESS;
}
