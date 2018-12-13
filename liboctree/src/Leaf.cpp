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

#include "Leaf.hpp"

Leaf::Leaf(std::vector<float> const& data)
{
	for(unsigned int i(0); i < data.size(); i += 3)
	{
		if(data[i] < minX)
			minX = data[i];
		if(data[i] > maxX)
			maxX = data[i];
		if(data[i + 1] < minY)
			minY = data[i + 1];
		if(data[i + 1] > maxY)
			maxY = data[i + 1];
		if(data[i + 2] < minZ)
			minZ = data[i + 2];
		if(data[i + 2] > maxZ)
			maxZ = data[i + 2];

		if(data.size() <= 3 * MAX_LEAF_SIZE
		   || (static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
		          < (3 * MAX_LEAF_SIZE) / (float) data.size())
		{
			this->data.push_back(data[i]);
			this->data.push_back(data[i + 1]);
			this->data.push_back(data[i + 2]);
		}
	}
}

Leaf::Leaf(std::istream& in)
{
	brw::read(in, file_addr);
}

Leaf::Leaf(long file_addr)
    : file_addr(file_addr)
{
}

void Leaf::writeData(std::ostream& out)
{
	file_addr = out.tellp();
	brw::write(out, minX);
	brw::write(out, maxX);
	brw::write(out, minY);
	brw::write(out, maxY);
	brw::write(out, minZ);
	brw::write(out, maxZ);
	brw::write(out, data);
}

void Leaf::readData(std::istream& in)
{
	in.seekg(file_addr);
	brw::read(in, minX);
	brw::read(in, maxX);
	brw::read(in, minY);
	brw::read(in, maxY);
	brw::read(in, minZ);
	brw::read(in, maxZ);
	brw::read(in, data);
}

std::string Leaf::toString(std::string const& tabs) const
{
	std::ostringstream oss;
	for(unsigned int i(0); i < data.size(); i += 3)
		oss << tabs << data[i] << "; " << data[i + 1] << "; " << data[i + 2]
		    << std::endl;
	return oss.str();
}
