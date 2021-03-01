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

#include <array>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <vector>

#include "binaryrw.hpp"

#define MAX_LEAF_SIZE 16000
#define MAX_THREADS 8

#define VERSION_MAJOR 2
#define VERSION_MINOR 0

/*! \mainpage
 *
 * This library is about constructing, reading and writing octrees in a way that
 * is efficient for the VIRUP project. (But it should be possible to use it
 * elsewhere if needed.)
 * All the external interactions with the lib should involve the \ref Octree
 * class. Among those interactions, you can use it to create octrees from
 * position data, read an octree from an octree file or write an octree to an
 * octree file. This class was designed to be inherited, so that you can extend
 * it with other useful features (like OpenGL calls for drawing in the case of
 * VIRUP for example).<br/>
 *
 * The more low-level side of the library was not designed to be useful from
 * outside but is exposed through the \ref brw namespace if needed. It is a
 * collection of functions to read/write binary data efficiently and is
 * extensively used by the \ref Octree class to read/write octrees.
 */

/*! \brief Octree main class
 */
class Octree
{
  public:
	/*! \brief Holds information about the octree.
	 *
	 * This information concerns the octree itself or the data it holds (which
	 * kind of data, how it is stored, etc...).
	 */
	enum class Flags : uint64_t
	{
		// MODIFIERS
		/*! \brief Null flags, for comparison or initialization.
		 */
		NONE = 0x0000000000000000ULL,
		/*! \brief Set if the positions are stored from (0, 0, 0) to (1, 1, 1)
		 * relative to the node.
		 *
		 * The bounding box is still absolute and can be used to retrieve
		 * absolute coordinates for the data.
		 */
		NORMALIZED_NODES = 0x0000000000000001ULL,
		/*! \brief Created with a stored version or not.
		 *
		 * It's entirely handled within the octree, so setting or unsetting from
		 * outside Octree's code has no effect.
		 */
		VERSIONED = 0x0000000000000002ULL,

		// DATA TYPES STORED
		/*! \brief Set if the particles radii are also stored.
		 */
		STORE_RADIUS = 0x0000000100000000ULL,
		/*! \brief Set if the particles luminosities are also stored.
		 */
		STORE_LUMINOSITY = 0x0000000200000000ULL,
		/*! \brief Set if the particles colors are also stored.
		 */
		STORE_COLOR = 0x0000000400000000ULL,
	};

	struct CommonData
	{
		uint32_t versionMajor = VERSION_MAJOR;
		uint32_t versionMinor = VERSION_MINOR;

		Flags flags = Flags::NONE;
		/*! @brief Number of dimensions per vertex (3 by default with only 3D
		 * positions).
		 */
		unsigned int dimPerVertex = 3;
	};

	/*! \brief Constructs an empty root node
	 *
	 * Call setFlags if necessary, then init to populate the tree.
	 */
	Octree();

	/*! \brief Returns this octree's \ref Flags
	 */
	Flags getFlags() const { return commonData.flags; };

	/*! \brief Sets this octree's \ref Flags
	 *
	 * \attention Be careful, changing flags when data is already loaded can
	 * have dramatic consequences.
	 */
	void setFlags(Flags flags);

	/*! \brief Initializes the octree from position data.
	 *
	 * It will also compute all the mins and maxes.
	 *
	 * \warning The \p data vector will get \e emptied by the octree as it
	 * copies data from it to prevent excessive memory usage.
	 *
	 * \param data : vector holding positions, structured as follows for N
	 * points : {x1, y1, z1, ... xN, yN, zN}.
	 */
	virtual void init(std::vector<float>& data);

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
	 * \param in : File stream from which the node is read.
	 */
	virtual void init(int64_t file_addr, std::istream& in);

	/*! \brief Size of the total non-redundant data stored in the tree.
	 *
	 * It is effectively the sum of the data sizes of all the leaves.
	 * This is computed during init() so should always be valid after calling
	 * init().
	 */
	size_t getTotalDataSize() const { return totalDataSize; };

	/*! \brief Tests if this node is in fact a leaf.
	 *
	 * More explicitly, returns true if and only if all elements of \ref
	 * children are nullptr.
	 */
	bool isLeaf() const;

	/*! \brief Returns a "string" of 64bit ints that represents the octree's
	 * structure
	 *
	 * This vector complies to the octree file format and can be written as is
	 * to represent a TREE from the grammar. If this is the root node, don't
	 * forget not to write the size of the vector and the first parenthesis to
	 * fully comply to the format.
	 */
	virtual std::vector<int64_t> getCompactData() const;

	/*! \brief Writes the data within the stream and updates file_addr
	 * accordingly.
	 *
	 * It will write all min/maxes and the position data.
	 * \param out : stream to which to write
	 */
	virtual void writeData(std::ostream& out);

	/*! \brief Reads data at file_addr from a stream
	 *
	 * It will read all min/maxes and the position data and ask its children (if
	 * any) to read their data also.
	 * \param in : stream from which to read
	 */
	virtual void readData(std::istream& in);

	/*! \brief Reads data at file_addr from a stream
	 *
	 * It will read all min/maxes and the position data but won't ask its
	 * children to read their data.
	 * Mostly useful for dynamic loading.
	 * \param in : stream from which to read
	 */
	virtual void readOwnData(std::istream& in);

	/*! \brief Reads only bounding box related data.
	 *
	 * Reads all min/maxes starts at file_addr and ask its children (if any)
	 * to also do so.
	 * \param in : stream from which to read
	 */
	virtual void readBBoxes(std::istream& in);

	/*! \brief Reads only bounding box related data.
	 *
	 * Reads all min/maxes starts at file_addr but won't ask its children (if
	 * any) to also do so.
	 * \param in : stream from which to read
	 */
	virtual void readBBox(std::istream& in);

	/*! \brief Returns the position data only contained within this node.
	 */
	virtual std::vector<float> getOwnData() const;

	/*! \brief Returns all the position data contained within the whole octree.
	 */
	virtual std::vector<float> getData() const;

	/*! \brief Returns a displayable string to represent the tree.
	 */
	virtual std::string toString(std::string const& tabs = "") const;

	/*! \brief Destructor.
	 *
	 * Deletes children if any.
	 */
	virtual ~Octree();

	/*! \brief Helper function to show and update a progress bar in the terminal
	 *
	 * \param progress : from 0.0 (0%) to 1.0 (100%)
	 */
	static void showProgress(float progress);

	// Root node allocates or frees common data for the whole tree
	// and is the only node not to store nullptr in this attribute.
	CommonData* rootManagedCommonData = nullptr;

  protected:
	CommonData& commonData;

	/*! \brief Constructs a child with common data.
	 */
	Octree(CommonData& commonData);
	/*! \brief Constructs a new child.
	 *
	 * Useful for inheritence and polymorphism. You SHOULD reimplement this
	 * method if you inherit from Octree to return your own instance of it. This
	 * is needed due to the recursive nature of this class. When Octree
	 * initializes itself, it will call newOctree to create children to ensure
	 * the whole tree will be of the right class.
	 *
	 * \attention Don't forget to call the child constructor taking a CommonData
	 * reference.
	 *
	 * Example : inheriting from Octree
	 * @code
	 * class MyOctree : public Octree
	 * {
	 *    // ... some members ...
	 *    protected:
	 *      virtual Octree* newChild() const
	 *      {
	 *          return new MyOctree(commonData);
	 *      };
	 * }
	 * @endcode
	 */
	virtual Octree* newChild() const;

	/*! \brief Address within a file where lies or should lie the data.
	 */
	int64_t file_addr = -2;

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

	/*! \brief Size of data stored in leaves
	 *
	 * This is not the size of the data attribute, but the size of the data
	 * stored in the tree, without redundancy. Can be read independently from
	 * data.
	 */
	size_t totalDataSize = 0;

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
	std::array<Octree*, 8> children = {{nullptr, nullptr, nullptr, nullptr,
	                                    nullptr, nullptr, nullptr, nullptr}};

  private:
	/*! \brief Converts bounding box to 3 uint_64t.
	 *
	 * Used to represent the bounding box in compact data.
	 */
	std::array<uint64_t, 3> getBoundingBoxUint64Representation() const;

	// Data indices go from 0 to data.size()-1.
	// Vertices indices go from 0 to data.size()/3 - 1 (a triplet of values is
	// ONE vertex).

	// init helper that only uses data from beg to end (included).
	// beg and end are vertices indices.
	void init(std::vector<float>& data, size_t beg, size_t end);

	// Gets a vertex's component from data.
	// vertex is the vertex's index.
	// (get(data, 10, 1) will return the y component of the 11th vertex.)
	float get(std::vector<float> const& data, size_t vertex, unsigned int dim);
	// Sets a vertex component in data (see get for indexing).
	void set(std::vector<float>& data, size_t vertex, unsigned int dim,
	         float val);
	// Swaps two vertices from data, the (i+1)th and the (j+1)th.
	// This swaps all components of the vertices (x, y, z).
	void swap(std::vector<float>& data, size_t i, size_t j);
	// Considers only dim's component of vertices from vertex indices beg to
	// end. All vertices which value is bellow pivot will be sorted to be before
	// all vertices which value is above or equal to pivot. This is a partial
	// sort that splits the data into two parts : one below pivot and one above.
	// This partial sort is O(end-beg).
	// The split's index is returned and is the first index of the second part.
	//
	// ex :
	// orderPivot({4, x, x, 5, x, x, 1, x, x, 2, x, x, 3, x, x, 0, x, x, 6,
	// x, x}, 0, 6, 0, 3.5)
	// will reorder the data as :
	// {0, x, x, 3, x, x, 1, x, x, 2, x, x, 5, x, x 4, x, x, 6, x, x}
	// and return 4
	// x aren't important for this example but they are the other two components
	// of the vertices and will be moved along with the first component.
	// Vertices data is always conserved and triplets are always moved together.
	size_t orderPivot(std::vector<float>& data, size_t beg, size_t end,
	                  unsigned int dim, float pivot);

	// to write LIBOCTREE.debug which holds the ASCII-translated compact data of
	// the tree
	static std::string tabs;
	static std::ofstream debug;

	static size_t totalNumberOfVertices;

	// versioned file reading methods
	void init1_0(std::istream& in);
	void init2_0(std::istream& in);
	void init1_0(int64_t file_addr, std::istream& in);
	void init2_0(int64_t file_addr, std::istream& in);
	void readOwnData1_0(std::istream& in);
	void readOwnData2_0(std::istream& in);
	void readBBox1_0(std::istream& in);
	void readBBox2_0(std::istream& in);
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

/*! \brief Returns bitwise NOT value of a flag, considering it is equivalent to
 * uint64_t.
 */
Octree::Flags operator~(Octree::Flags f);

/*! \brief Returns bitwise OR value between two Octree#Flags considering they
 * are equivalent to uint64_t.
 */
Octree::Flags operator|(Octree::Flags a, Octree::Flags b);

/*! \brief Stores the result of \p a | \p b in \p a.
 */
Octree::Flags& operator|=(Octree::Flags& a, Octree::Flags b);

/*! \brief Returns bitwise AND value between two Octree#Flags considering they
 * are equivalent to uint64_t.
 */
Octree::Flags operator&(Octree::Flags a, Octree::Flags b);

/*! \brief Stores the result of \p a & \p b in \p a.
 */
Octree::Flags& operator&=(Octree::Flags& a, Octree::Flags b);

#endif // OCTREE_H
