[![pipeline status](https://gitlab.com/Dexter9313/octree-file-format/badges/master/pipeline.svg)](https://gitlab.com/Dexter9313/octree-file-format/commits/master)

# octree-file-format

A collection of tools to handle the *.octree* file format and convert to it from HDF5. This file format is designed to efficiently store octrees to be used as input for [virup-prototype](https://gitlab.com/Dexter9313/virup-prototype).

This format was not designed to be optimal regarding disk usage (even if it is close to optimality in most cases [~= 0.1% more than necessary space used in practice]), but rather to be the closest representation of a VIRUP rendering octree as possible when loaded in memory during runtime. This is such that dynamically loading a node while rendering should be as fast as possible and shouldn't require zigzagging within the file.

The principle is the following : the structure of the tree is stored in the file's header. Data is stored after that in chunks. Each chunk corresponds to one node (or leaf) data. The tree structure holds pointers (byte addresses within the file) to these chunks, such that each node (or leaf) can retrieve its data with random access. The normal flow would then be : load the empty tree by creating its structure, each node/leaf holding a pointer within the still open file; then, when needed, each node can load/unload its data by reading the file from this pointer.

What makes separating structure from data worth is that VIRUP octrees aren't full depth octrees. Each leaf stores around 16000 tri-dimensional points, and each node stores a subsample (also of size 16000) of its descendants points for Level of Detail. Storing the structure of the tree can be for example a megabyte worth and the data stored in this tree will be worth a few dozen gigabytes. This way, initial loading of the tree structure is done almost instantly without having to parse the whole file.

## Tools

Please read each tool's README inside its own directory !

* [liboctree](https://gitlab.com/Dexter9313/octree-file-format/blob/master/liboctree/) : Library that can build octrees, or read/write them from/to *.octree* files used by virup.
* [octreegen](https://gitlab.com/Dexter9313/octree-file-format/blob/master/octreegen/) : Generates an octree file either from an HDF5 file or from a number to random particles to generate.
* [octreegen-gui](https://gitlab.com/Dexter9313/octree-file-format/blob/master/octreegen-gui/) : A user-friendly GUI interface for octreegen.

## Complete Grammar Syntax and hints on semantics

If you wish to write your own octree file format reader/writer or edit this project, here is the complete grammar of the format.
*The file format uses little-endian convention.*

### Terminal terms (vocabulary)

FLOAT   : a 32-bit floating point value

SIZE    : a 32-bit unsigned integer, usually the size of an array stored in a chunk (not counting bytes, but counting individual whole values, an array of four 32-bit value would be of SIZE 4)

(       : a 64-bit constant 0x0000000000000000 (0)

)       : a 64-bit constant 0x0000000000000001 (1)

null    : a 64-bit constant 0xFFFFFFFFFFFFFFFF (-1)

ADDRESS : a 64-bit integer that is not equal to either (, ) or null; usually a pointer to a CHUNK (e.g. if ADDRESS is 0x0000000000000002, this means the chunk starts from the third byte of the file)

empty   : void, defined for grammar syntax, represented by absolutly no bits in the file

### Non-terminal terms (rules)

S (axiom)    -> STRUCTURE CHUNKS

Separation of structure and chunks


STRUCTURE    -> SIZE NODE

The structure size (in 64-bit units, not in bytes - every value in the rest of the structure is represented as 64 bits) followed by the description of the root node.


NODE         -> ( ADDRESS TREE{,8} )

A node is surrounded by parenthesis (0 and 1 in 64-bit format). The first value it holds is the address of its corresponding chunk. Then it can have none to eight children. The order of defined children is important and is the same accross all trees in the file (hence the existance of the "null" tree to leave spaces if necessary).


TREE         -> NODE | LEAF | null

A tree can be either a node, a leaf or a "null" tree (absence of tree).


LEAF         -> ADDRESS

A leaf only consists of the address of its corresponding chunk.


CHUNKS       -> CHUNK CHUNKS | CHUNK

Chunks are specified contiguously and there has to be at least one (for the root node).


CHUNK        -> BOUNDING_BOX SIZE DATA

A chunk is contiguously described by a bounding box, then the size of its data (counting the 32-bit values, not the bytes), and then the data itself.


BOUNDING_BOX -> FLOAT FLOAT FLOAT FLOAT FLOAT FLOAT

A bounding box consists of 6 32-bit floating point values. These values represent the bounding box of the data held by the chunk, in this order : minX, maxX, minY, maxY, minZ, maxZ. This small data redundancy helps shortening loading times in practice.


DATA         -> POSITION DATA | empty

The data is a contiguous list of positions. There can be none; in this case, the corresponding SIZE must be zero and the BOUNDING_BOX will hold invalid data.


POSITION     -> FLOAT FLOAT FLOAT

A position is a contiguous triplet of 32-bit floating point values representing a point's coordinates (x, y then z).


### Example

To represent this tree (D is the data of the node and 0-7 are its children):

	D:0.1 0.2 0.3
	0:
		D:0.4 0.5 0.6
		0:
		1:
		2:
			D:0.7 0.8 0.9
		3:
		4:
		5:
		6:
		7:
	1:
		D:1.0 1.1 1.2
		  1.3 1.4 1.5
	2:
	3:
	4:
	5:
	6:
	7:

The corresponding file would contain the following values :

	10
	0
	CHUNK0-ADDR
	0
	CHUNK1-ADDR
	-1
	-1
	CHUNK2-ADDR
	1
	CHUNK3-ADDR
	1
	//CHUNK0
	0.1
	0.1
	0.2
	0.2
	0.3
	0.3
	3
	0.1
	0.2
	0.3
	//CHUNK1
	0.4
	0.4
	0.5
	0.5
	0.6
	0.6
	3
	0.4
	0.5
	0.6
	//CHUNK2
	0.7
	0.7
	0.8
	0.8
	0.9
	0.9
	3
	0.7
	0.8
	0.9
	//CHUNK3
	1.0
	1.3
	1.1
	1.4
	1.2
	1.5
	6
	1.0
	1.1
	1.2
	1.3
	1.4
	1.5

All values starting with "//" are not in the file but are comments for this documentation.
Also, CHUNKi-ADDR values should hold the true address of CHUNK i in bytes (e.g. CHUNK0-ADDR = 4 + 10x8 = 84 because it's the 12th value written in the file after one 4-bytes value and ten 8-bytes values), but for readability purposes, it was replaced by these "constant names".
