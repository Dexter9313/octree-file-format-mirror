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

#include <liboctree/Octree.hpp>
#include <liboctree/binaryrw.hpp>

#include "utils.hpp"

namespace term
{
const std::string red("\033[31;31m");
const std::string green("\033[32;32m");
const std::string reset("\033[0;0m");
} // term

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

int main(int, char* [])
{
	const unsigned int seed = 0;
	srand(seed);
	std::string success(term::green + "[SUCCESS] " + term::reset);
	std::string failure(term::red + "[FAILURE] " + term::reset);
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
