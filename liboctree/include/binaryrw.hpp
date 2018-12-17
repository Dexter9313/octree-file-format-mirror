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

#ifndef BINARYRW_H
#define BINARYRW_H

#include <fstream>
#include <iostream>
#include <vector>

/*! \brief Binary read-write.
 *
 * This namespace contains simple functions to read and write binary data from
 * and to a file.
 *
 * Reads and writes are supposed to be used together : some data written with
 * one of these "write" functions should be read with its correponding "read"
 * function. Other usage behaviors are possible but risky, be cautious !
 */
namespace brw
{
// DECLARATIONS
/*! \brief Returns true if and only if the machine this function is running on
 * is
 * little endian.
 */
inline bool isLittleEndian();

/*! \brief Writes some base-type data.
 *
 * Example 1 : writing an integer to a file
 * @code
 * std::ofstream f("/path", std::fstream::out | std::fstream::binary);
 * int x(42);
 * brw::write(f, x);
 * f.close();
 * @endcode
 *
 * Example 2 : writing an integer buffer to a file
 * @code
 * std::ofstream f("/path", std::fstream::out | std::fstream::binary);
 * int x[2] = {42, 43};
 * brw::write(f, x[0], 2);
 * f.close();
 * @endcode
 *
 * \param stream : the stream in which to write
 * \param x : the buffer of base-type T to write
 * \param n : if x is the first element of a buffer, which is the number of
 * elements to write
 */
template <typename T>
inline void write(std::ostream& stream, T& x, unsigned int n = 1);

/*! \brief Reads some base-type data.
 *
 * Example 1 : reading an integer from a file
 * @code
 * std::ifstream f("/path", std::fstream::in | std::fstream::binary);
 * int x;
 * brw::read(f, x);
 * // x now holds the read value
 * f.close();
 * @endcode
 *
 * Example 2 : reading an integer buffer from a file
 * @code
 * std::ifstream f("/path", std::fstream::in | std::fstream::binary);
 * int x[2];
 * brw::read(f, x[0], 2);
 * // x now holds the two read values
 * f.close();
 * @endcode
 *
 * \param stream : the stream from which to read
 * \param res : the data of base-type T to write into once it's read
 * \param n : if res is the first element of a buffer, which is the number of
 * elements to read
 */
template <typename T>
inline void read(std::istream& stream, T& res, unsigned int n = 1);

/*! \brief Writes a vector of base-type.
 *
 * The size of the vector will also be written, which is useful for reading.
 *
 * Example : writing a vector of integers to a file
 * @code
 * std::ofstream f("/path", std::fstream::out | std::fstream::binary);
 * std::vector<int> v = {1, 2, 3};
 * brw::write(f, v);
 * f.close();
 * @endcode
 *
 * \param stream : the stream in which to write
 * \param array : the vector of base-type T to write
 */
template <typename T>
inline void write(std::ostream& stream, std::vector<T>& array);

/*! \brief Reads a vector of base-type.
 *
 * The size of the vector will be read from the file.
 *
 * Example : reading a vector of integers from a file
 * @code
 * std::ifstream f("/path", std::fstream::in | std::fstream::binary);
 * std::vector<int> v;
 * brw::read(f, v);
 * // v now holds the read vector
 * f.close();
 * @endcode
 *
 * \param stream : the stream from which to read
 * \param vec : the vector of base-type T in which to write the read data
 */
template <typename T>
inline void read(std::istream& stream, std::vector<T>& vec);

/*! \brief \brief Reads a whole stream and writes the hex values found to
 * another.
 *
 * It will read \p stream char by char and write their corresponding hex values
 * one by one.
 *
 * \param stream : the stream from which to read the data
 * \param dbgstream : the stream to which to write the hex values
 */
inline void debugread(std::istream& stream, std::ostream& dbgstream);

// DEFINITIONS
bool isLittleEndian()
{
	uint32_t x(1);
	return reinterpret_cast<unsigned char*>(&x)[0];
}

template <typename T>
void write(std::ostream& stream, T& x, unsigned int n)
{
	if(isLittleEndian())
		stream.write(reinterpret_cast<char*>(&x), n * sizeof(T));
	else
		for(int i(n * sizeof(T) - 1); i >= 0; --i)
			stream.write(&(reinterpret_cast<char*>(&x)[i]), 1);
}

template <typename T>
void read(std::istream& stream, T& res, unsigned int n)
{
	char* buff = reinterpret_cast<char*>(&res);
	if(isLittleEndian())
		stream.read(buff, n * sizeof(T));
	else
		for(int i(n * sizeof(T) - 1); i >= 0; --i)
			stream.read(&buff[i], 1);
}

template <typename T>
void write(std::ostream& stream, std::vector<T>& vec)
{
	uint32_t size(vec.size());
	write(stream, size);
	write(stream, vec[0], vec.size());
}

template <typename T>
void read(std::istream& stream, std::vector<T>& vec)
{
	uint32_t size;
	read(stream, size);
	vec.resize(size);
	read(stream, vec[0], size);
}

void debugread(std::istream& stream, std::ostream& dbgstream)
{
	char x;
	while(!stream.eof())
	{
		stream.read(&x, 1);
		dbgstream << std::hex << *reinterpret_cast<uint32_t*>(&x) << " ";
	}
	dbgstream << std::endl;
}

} // ns brw

#endif // BINARYRW_H
