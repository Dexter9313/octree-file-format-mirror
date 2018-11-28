#ifndef LEAF_H
#define LEAF_H

#include <cfloat>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <vector>

#include "binaryrw.hpp"

#define MAX_LEAF_SIZE 16000

class Leaf
{
  public:
	Leaf(std::vector<float> const& data);
	Leaf(std::ifstream& in);
	virtual std::vector<long> getCompactData() const
	{
		return std::vector<long>({file_addr});
	};
	virtual void writeData(std::ofstream& out);
	virtual void readData(std::ifstream& in);
	virtual void debug(std::string const& tabs) const;
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
