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

#ifndef OCTREE_H
#define OCTREE_H

#include <cfloat>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "binaryrw.hpp"

#define MAX_LEAF_SIZE 16000

class Octree
{
  public:
	/*! Data constructor
	 *
	 * Constructs an octree from cartesian positions.
	 * \param data Vector of the form {x0, y0, z0, x1, y1, z1, ... , xn, yn,
	 * zn}.
	 */
	Octree(std::vector<float> data);
	Octree(std::istream& in);
	Octree(long file_addr);
	bool isLeaf() const;
	virtual std::vector<long> getCompactData() const;
	virtual void writeData(std::ostream& out);
	virtual void readData(std::istream& in);
	virtual std::string toString(std::string const& tabs = "") const;
	virtual ~Octree();

  protected:
	virtual Octree* newOctree(std::vector<float> data) const;
	virtual Octree* newOctree(std::istream& in) const;
	virtual Octree* newOctree(long file_addr) const;

	long file_addr = -2;

	float minX = FLT_MAX;
	float maxX = -FLT_MAX;
	float minY = FLT_MAX;
	float maxY = -FLT_MAX;
	float minZ = FLT_MAX;
	float maxZ = -FLT_MAX;

	std::vector<float> data;
	Octree* children[8] = {nullptr, nullptr, nullptr, nullptr,
	                     nullptr, nullptr, nullptr, nullptr};
};

void write(std::ostream& stream, Octree& octree);

#endif // OCTREE_H
