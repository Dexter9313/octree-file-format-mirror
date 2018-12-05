#ifndef OCTREE_H
#define OCTREE_H

#include "Leaf.hpp"

class Octree : public Leaf
{
  public:
	Octree(std::vector<float> data);
	Octree(std::istream& in);
	virtual std::vector<long> getCompactData() const;
	virtual void writeData(std::ostream& out);
	virtual void readData(std::istream& in);
	virtual std::string toString(std::string const& tabs = "") const;
	virtual ~Octree();

  private:
	Leaf* children[8] = {nullptr, nullptr, nullptr, nullptr,
	                     nullptr, nullptr, nullptr, nullptr};
};

void write(std::ostream& stream, Octree& octree);

#endif // OCTREE_H
