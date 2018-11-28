#ifndef OCTREE_H
#define OCTREE_H

#include "Leaf.hpp"

class Octree : public Leaf
{
  public:
	Octree(std::vector<float> data);
	Octree(std::ifstream& in);
	virtual std::vector<long> getCompactData() const;
	virtual void writeData(std::ofstream& out);
	virtual void readData(std::ifstream& in);
	virtual void debug(std::string const& tabs = "") const;
	virtual ~Octree();

  private:
	Leaf* children[8] = {nullptr, nullptr, nullptr, nullptr,
	                     nullptr, nullptr, nullptr, nullptr};
};

void write(std::ofstream& stream, Octree& octree);

#endif // OCTREE_H
