#include "binaryrw.hpp"

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
