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

/*! \brief Octree main class
 */
class Octree
{
  public:
	/*! \brief Initializes the octree from position data.
	 *
	 * It will also compute all the mins and maxes.
	 *
	 * \param data : vector holding positions, structured as follows for N
	 * points : {x1, y1, z1, ... xN, yN, zN}.
	 */
	virtual void init(std::vector<float> data);

	/*! \brief Initializes the octree from a stream.
	 *
	 * The tree will only read its structure and not its data. To read the data,
	 * call \ref readData afterwards.
	 *
	 * \param in : stream from which to read the structure
	 */
	virtual void init(std::istream& in);

	/*! \brief Initializes the octree from a address within a file.
	 *
	 * Typically called by init(std::istream&) when reading the structure if
	 * this octree to be created is a leaf.
	 * No children will be created, only the file address will be set.
	 *
	 * \param file_addr : Address within the file where the data lies.
	 */
	virtual void init(long file_addr);

	/*! \brief Tests if this node is in fact a leaf.
	 *
	 * More explicitly, returns true if and only if all elements of \ref
	 * children are nullptr.
	 */
	bool isLeaf() const;

	/*! \brief Returns a "string" of long ints that represents the octree's
	 * structure
	 *
	 * This vector complies to the octree file format and can be written as is
	 * to represent a TREE from the grammar. If this is the root node, don't
	 * forget not to write the size of the vector and the first parenthesis to
	 * fully comply to the format.
	 */
	virtual std::vector<long> getCompactData() const;

	/*! \brief Writes the data within the stream and updates file_addr
	 * accordingly.
	 *
	 * It will write all min/maxes and the position data.
	 * \param out : stream to which to write
	 */
	virtual void writeData(std::ostream& out);

	/*! \brief Reads data at file_addr from a stream
	 *
	 * It will read all min/maxes and the position data.
	 * \param in : stream from which to read
	 */
	virtual void readData(std::istream& in);

	/*! \brief Returns all the position data contained within the whole octree.
	 */
	virtual std::vector<float> getAllData() const;

	/*! \brief Returns a displayable string to represent the tree.
	 */
	virtual std::string toString(std::string const& tabs = "") const;

	/*! \brief Destructor.
	 *
	 * Deletes children if any.
	 */
	virtual ~Octree();

  protected:
	/*! \brief Constructs a new octree.
	 *
	 * Useful for inheritence and polymorphism. You SHOULD reimplement this
	 * method if you inherit from Octree to return your own instance of it. This
	 * is needed due to the recursive nature of this class. When Octree
	 * initializes itself, it will call newOctree() to create children to ensure
	 * the whole tree will be of the right class.
	 *
	 * Example : inheriting from Octree
	 * @code
	 * class MyOctree : public Octree
	 * {
	 *    // ... some members ...
	 *    protected:
	 *      virtual Octree* newOctree() const { return new MyOctree(); };
	 * }
	 * @endcode
	 */
	virtual Octree* newOctree() const;

	/*! \brief Address within a file where lies or should lie the data.
	 */
	long file_addr = -2;

	/*! \brief Min value of the positions' x component.
	 *
	 * Part of the data read/written.
	 */
	float minX = FLT_MAX;
	/*! \brief Max value of the positions' x component.
	 *
	 * Part of the data read/written.
	 */
	float maxX = -FLT_MAX;
	/*! \brief Min value of the positions' y component.
	 *
	 * Part of the data read/written.
	 */
	float minY = FLT_MAX;
	/*! \brief Max value of the positions' y component.
	 *
	 * Part of the data read/written.
	 */
	float maxY = -FLT_MAX;
	/*! \brief Min value of the positions' z component.
	 *
	 * Part of the data read/written.
	 */
	float minZ = FLT_MAX;
	/*! \brief Max value of the positions' z component.
	 *
	 * Part of the data read/written.
	 */
	float maxZ = -FLT_MAX;

	/*! \brief Position data.
	 *
	 * Part of the data read/written.
	 *
	 * This vector is structured as follows for N points : {x1, y1, z1, ... xN,
	 * yN, zN}.
	 */
	std::vector<float> data;

	/*! \brief Children of this node.
	 *
	 * If this node is a leaf, all the children will be nullptr.
	 */
	Octree* children[8] = {nullptr, nullptr, nullptr, nullptr,
	                       nullptr, nullptr, nullptr, nullptr};
};

/*! \brief Writes an Octree in a stream.
 *  \relates Octree
 *
 * It will write the whole file structure (tree structure + data chunks) within
 * the stream according to the octree file format.
 *
 * \param stream : stream in which to write
 * \param octree : octree to be written
 */
void write(std::ostream& stream, Octree& octree);

#endif // OCTREE_H
