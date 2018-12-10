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

#ifndef LEAF_H
#define LEAF_H

#include <cfloat>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "binaryrw.hpp"

#define MAX_LEAF_SIZE 16000

class Leaf
{
  public:
	Leaf(std::vector<float> const& data);
	Leaf(std::istream& in);
	virtual std::vector<long> getCompactData() const
	{
		return std::vector<long>({file_addr});
	};
	virtual void writeData(std::ostream& out);
	virtual void readData(std::istream& in);
	virtual std::string toString(std::string const& tabs) const;
	virtual ~Leaf(){};

  protected:
	long file_addr = -2;

	float minX = FLT_MAX;
	float maxX = -FLT_MAX;
	float minY = FLT_MAX;
	float maxY = -FLT_MAX;
	float minZ = FLT_MAX;
	float maxZ = -FLT_MAX;
	std::vector<float> data;
};

#endif // LEAF_H
