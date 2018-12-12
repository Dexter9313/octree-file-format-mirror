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

/*! Binary read-write.
 */
namespace brw
{
// DECLARATIONS
/*! Returns true if and only if the machine this function is running on is little endian.
 */
inline bool isLittleEndian();

template <typename T>
inline void write(std::ostream& stream, T& x, unsigned int n = 1);

template <typename T>
inline void read(std::istream& stream, T& res, unsigned int n = 1);

template <typename T>
inline void write(std::ostream& stream, std::vector<T>& array);

template <typename T>
inline void read(std::istream& stream, std::vector<T>& vec);

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
