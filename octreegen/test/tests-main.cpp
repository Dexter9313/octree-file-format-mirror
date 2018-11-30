#include "Octree.hpp"
#include "binaryrw.hpp"
#include "utils.hpp"

#include <fstream>
#include <iostream>
#include <vector>

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
		Octree octree1(generateVertices(100000, seed));
		TestBinaryFile f;
		f.resetCursor();
		write(f, octree1);
		f.resetCursor();
		uint32_t foo;
		long bar;
		brw::read(f, foo);
		brw::read(f, bar);
		Octree octree2(f);
		octree2.readData(f);
		TEST_EQUAL(octree1.toString(), octree2.toString(), "R/W random octree");
		std::cout << success << "R/W random octree" << std::endl;
	}
	// TEST String splitting
	{
		std::string str("This is a sentence.");
		std::vector<std::string> result(split(str, ' '));
		TEST_EQUAL(result[0], std::string("This"), "String splitting");
		TEST_EQUAL(result[1], std::string("is"), "String splitting");
		TEST_EQUAL(result[2], std::string("a"), "String splitting");
		TEST_EQUAL(result[3], std::string("sentence."), "String splitting");
		std::cout << success << "String splitting" << std::endl;
	}
	// TEST String joining
	{
		std::string str("This is a sentence.");
		std::string result(join(split(str, ' '), ' '));
		TEST_EQUAL(result, str, "String joining");
		std::cout << success << "String joining" << std::endl;
	}

	return EXIT_SUCCESS;
}