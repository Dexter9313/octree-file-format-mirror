#ifndef BINARYRW_H
#define BINARYRW_H

#include <fstream>
#include <iostream>
#include <vector>

namespace brw
{
inline bool isLittleEndian();

template <typename T>
inline void write(std::ostream& stream, T& x, unsigned int n = 1);

template <typename T>
inline void read(std::istream& stream, T& res, unsigned int n = 1);

template <typename T>
inline void write(std::ostream& stream, std::vector<T>& array);

template <typename T>
inline void read(std::istream& stream, std::vector<T>& vec);

inline void debugread(std::istream& stream, std::ostream& dbgstream);

#include "binaryrw.cpp"
} // ns brw

#endif // BINARYRW_H
