/*
    Copyright (C) 2023 Florian Cabot <florian.cabot@hotmail.fr>

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

#ifndef HANDLE_ARGUMENTS_HPP
#define HANDLE_ARGUMENTS_HPP

#include <string>
#include <vector>

namespace arg
{

// global
enum class Command
{
	INVALID,
	HELP,
	INFO,
	GENERATE
};

// invalid
struct InvalidArguments
{
	std::string wrongCommand;
};
// end invalid

// info
enum class InfoSubCommand
{
	INVALID,
	HELP,
	INFO,
};

struct InfoArguments
{
	InfoSubCommand subcommand;
	std::string errorMessage;
	std::string input;
};
// end info

// generate
enum class GenerateSubCommand
{
	INVALID,
	HELP,
	GENERATE,
};

struct GenerateInputOptions
{
	float sampleRate = 1.f;
};

enum class GenerateInputType
{
	INVALID,
	RANDOM,
	OCTREE,
	HDF5,
};

struct GenerateRandomInputArgs
{
	unsigned int particlesNumber = 0;
	bool radius = false;
	bool lum = false;
	bool rgbLum = false;
	bool density = false;
	bool temperature = false;
};

struct GenerateOctreeInputArgs
{
	std::vector<std::string> octreeFiles;
};

struct GenerateHDF5InputArgs
{
	std::vector<std::string> hdf5Files;
	std::string coordPath;
	std::string radiusPath;
	std::string lumPath;
	std::string rgbLumPath;
	std::string densityPath;
	std::string temperaturePath;
};

struct GenerateOutputOptions
{
	bool normalizeNodes = true;
	unsigned int maxParticlesPerNode = 16000;
};

struct GenerateArguments
{
	GenerateSubCommand subcommand;
	std::string errorMessage;

	GenerateInputOptions inputOptions;
	GenerateInputType inputType;
	GenerateRandomInputArgs randomInputArgs;
	GenerateOctreeInputArgs octreeInputArgs;
	GenerateHDF5InputArgs hdf5InputArgs;

	GenerateOutputOptions outputOptions;
	std::string output;
};

// end generate

struct Arguments
{
	const Command command = Command::INVALID;
	void* subargs;

	Arguments(Arguments const& other)            = delete;
	Arguments& operator=(Arguments const& other) = delete;

	Arguments(Arguments&& other)
	    : command(other.command)
	    , subargs(other.subargs)
	    , doClean(other.doClean)
	{
		// prevent other from cleaning if it destroys itself
		other.doClean = false;
	};

	explicit Arguments(Command command)
	    : command(command)
	{
		switch(command)
		{
			case Command::INVALID:
				subargs = new InvalidArguments;
				break;
			case Command::INFO:
				subargs = new InfoArguments;
				break;
			case Command::GENERATE:
				subargs = new GenerateArguments;
				break;
			default:
				break;
		}
	}

	~Arguments()
	{
		if(!doClean)
		{
			return;
		}
		switch(command)
		{
			case Command::INVALID:
				delete static_cast<InvalidArguments*>(subargs);
				break;
			case Command::INFO:
				delete static_cast<InfoArguments*>(subargs);
				break;
			case Command::GENERATE:
				delete static_cast<GenerateArguments*>(subargs);
				break;
			default:
				break;
		}
		doClean = false;
	}

  private:
	bool doClean = true;
};

} // namespace arg

arg::Arguments handle_arguments(int argc, char* argv[]);

#endif // HANDLE_ARGUMENTS_HPP
