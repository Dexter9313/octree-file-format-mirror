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

#include "handle_arguments.hpp"

#include <iostream>

#include "utils.hpp"

arg::Arguments handle_info_arguments(std::vector<std::string> const& arguments)
{
	arg::Arguments result(arg::Command::INFO);
	auto& subargs = *static_cast<arg::InfoArguments*>(result.subargs);

	if(arguments.empty() || arguments[0] == "-h" || arguments[0] == "--help")
	{
		subargs.subcommand = arg::InfoSubCommand::HELP;
		return result;
	}

	if(arguments.size() > 1)
	{
		subargs.subcommand = arg::InfoSubCommand::INVALID;
		subargs.errorMessage = "Several files specified:\n";
		for(auto const& arg : arguments)
		{
			subargs.errorMessage += "-> " + arg + '\n';
		}
		return result;
	}

	subargs.subcommand = arg::InfoSubCommand::INFO;
	subargs.input = arguments[0];

	return result;
}

arg::Arguments handle_generate_arguments(std::vector<std::string> const& arguments)
{
	arg::Arguments result(arg::Command::GENERATE);
	auto& subargs = *static_cast<arg::GenerateArguments*>(result.subargs);

	if(arguments.empty() || arguments[0] == "-h" || arguments[0] == "--help")
	{
		subargs.subcommand = arg::GenerateSubCommand::HELP;
		return result;
	}
	subargs.subcommand = arg::GenerateSubCommand::GENERATE;

	enum class ParsingState
	{
		INPUT_OPTIONS,
		INPUT,
		OUTPUT_OPTIONS
	};
	ParsingState state(ParsingState::INPUT_OPTIONS);

	std::vector<std::string> inputOptionsStr, inputArgsStr, outputOptionsStr;
	for(unsigned int i(0); i < arguments.size(); ++i)
	{
		auto const& arg(arguments[i]);
		switch(state)
		{
			case ParsingState::INPUT_OPTIONS:
				if(arg == "--output")
				{
					subargs.subcommand = arg::GenerateSubCommand::INVALID;
					subargs.errorMessage = "No input specified.";
					return result;
				}
				if(arg == "--input-random" || arg == "--input-octree" || arg == "--input-hdf5")
				{
					state = ParsingState::INPUT;
					subargs.inputType = arg == "--input-random" ? arg::GenerateInputType::RANDOM :
						(arg == "--input-octree" ? arg::GenerateInputType::OCTREE :
						 arg::GenerateInputType::HDF5);
					break;
				}
				inputOptionsStr.push_back(arg);
				break;
			case ParsingState::INPUT:
				if(arg == "--output")
				{
					state = ParsingState::OUTPUT_OPTIONS;
					break;
				}
				inputArgsStr.push_back(arg);
				break;
			case ParsingState::OUTPUT_OPTIONS:
				// last element is OUTPUT
				if(i == arguments.size() - 1)
				{
					subargs.output = arg;
					break;
				}
				outputOptionsStr.push_back(arg);
				break;
		}
	}

	// CHECKS
	// Input option
	for(auto const& inOpt : inputOptionsStr)
	{
		auto s(split(inOpt, '='));
		if(s.size() != 2)
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Unknown input option: '" + inOpt + "'";
			return result;
		}
		if(s[0] != "--sample-rate")
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Unknown input option: '" + inOpt + "'";
			return result;
		}
		if(s[0] == "--sample-rate")
		{
			if(s[1].empty())
			{
				subargs.subcommand = arg::GenerateSubCommand::INVALID;
				subargs.errorMessage = "Invalid sample rate (empty).";
				return result;
			}
			for(char const& c : s[1])
			{
				if((c < '0' || c > '9') && c != '.')
				{
					subargs.subcommand = arg::GenerateSubCommand::INVALID;
					subargs.errorMessage = "Invalid sample rate (not a number): '" + s[1] + "'";
					return result;
				}
			}
			subargs.inputOptions.sampleRate = atof(s[1].c_str());
		}
	}
	// Input
	if(subargs.inputType == arg::GenerateInputType::RANDOM)
	{
		if(inputArgsStr.empty())
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Missing input argument (PARTICLES-NUMBER).";
			return result;
		}
		for(char const& c : inputArgsStr[0])
		{
			if(c < '0' || c > '9')
			{
				subargs.subcommand = arg::GenerateSubCommand::INVALID;
				subargs.errorMessage = "Invalid particles number (not an integer number): '" + inputArgsStr[0] + "'";
				return result;
			}
			subargs.randomInputArgs.particlesNumber = atoi(inputArgsStr[0].c_str());
		}
		for(unsigned int i(1); i < inputArgsStr.size(); ++i)
		{
			auto s = inputArgsStr[i];
			if(s == "--add-radius")
			{
				subargs.randomInputArgs.radius = true;
				continue;
			}
			if(s == "--add-lum")
			{
				subargs.randomInputArgs.lum = true;
				continue;
			}
			if(s == "--add-rgb-lum")
			{
				subargs.randomInputArgs.rgbLum = true;
				continue;
			}
			if(s == "--add-density")
			{
				subargs.randomInputArgs.density = true;
				continue;
			}
			if(s == "--add-temperature")
			{
				subargs.randomInputArgs.temperature = true;
				continue;
			}
			// else
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Unknown random input specifier: '" + s + "'";
			return result;
		}
	}
	else if(subargs.inputType == arg::GenerateInputType::OCTREE)
	{
		if(inputArgsStr.empty())
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Missing input argument (OCTREE-FILES).";
			return result;
		}
		for(auto const& f : inputArgsStr)
		{
			auto const& gl = glob(f);
			if(gl.empty())
			{
				subargs.subcommand = arg::GenerateSubCommand::INVALID;
				subargs.errorMessage = "Invalid file name or globbing expansion : '" + f + "'";
				return result;
			}
			for(auto const& fg : gl)
			{
				subargs.octreeInputArgs.octreeFiles.push_back(fg);
			}
		}
	}
	else if(subargs.inputType == arg::GenerateInputType::HDF5)
	{
		if(inputArgsStr.empty())
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Missing input argument (HDF5-FILES).";
			return result;
		}
		int foundCoordPath(-1);
		for(unsigned int i(0); i < inputArgsStr.size(); ++i)
		{
			auto s = inputArgsStr[i];
			if(s.substr(0, 13) == "--coord-path=")
			{
				if(i == 0)
				{
					subargs.subcommand = arg::GenerateSubCommand::INVALID;
					subargs.errorMessage = "Missing input argument (HDF5-FILES).";
					return result;
				}
				foundCoordPath = i;
				subargs.hdf5InputArgs.coordPath = split(s, '=')[1];
				break;
			}
			auto const& gl = glob(s);
			if(gl.empty())
			{
				subargs.subcommand = arg::GenerateSubCommand::INVALID;
				subargs.errorMessage = "Invalid file name or globbing expansion : '" + s + "'";
				return result;
			}
			for(auto const& fg : gl)
			{
				subargs.hdf5InputArgs.hdf5Files.push_back(fg);
			}
		}
		if(foundCoordPath == -1)
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Missing input argument --coord-path=<COORD-DATASET-PATH>";
			return result;
		}
		for(unsigned int i(foundCoordPath + 1); i < inputArgsStr.size(); ++i)
		{
			if(inputArgsStr[i].find('=') == std::string::npos)
			{
				subargs.subcommand = arg::GenerateSubCommand::INVALID;
				subargs.errorMessage = "Unknown hdf5 input specifier: '" + inputArgsStr[i] + "'";
				return result;
			}
			auto s = split(inputArgsStr[i], '=');
			if(s[0] == "--radius-path")
			{
				subargs.hdf5InputArgs.radiusPath = s[1];
				continue;
			}
			if(s[0] == "--lum-path")
			{
				subargs.hdf5InputArgs.lumPath = s[1];
				continue;
			}
			if(s[0] == "--rgb-lum-path")
			{
				subargs.hdf5InputArgs.rgbLumPath = s[1];
				continue;
			}
			if(s[0] == "--density-path")
			{
				subargs.hdf5InputArgs.densityPath = s[1];
				continue;
			}
			if(s[0] == "--temperature-path")
			{
				subargs.hdf5InputArgs.temperaturePath = s[1];
				continue;
			}
			//else
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Unknown hdf5 input specifier: '" + s[0] + "'";
			return result;
		}
	}
	// Output options
	for(auto const& outOpt : outputOptionsStr)
	{
		if(outOpt == "--disable-node-normalization")
		{
			subargs.outputOptions.normalizeNodes = false;
			continue;
		}
		auto s(split(outOpt, '='));
		if(s.size() != 2)
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Unknown output option: '" + outOpt + "'";
			return result;
		}
		if(s[0] != "--max-particles-per-node")
		{
			subargs.subcommand = arg::GenerateSubCommand::INVALID;
			subargs.errorMessage = "Unknown output option: '" + outOpt + "'";
			return result;
		}
		if(s[0] == "--max-particles-per-node")
		{
			if(s[1].empty())
			{
				subargs.subcommand = arg::GenerateSubCommand::INVALID;
				subargs.errorMessage = "Invalid max particles per node (empty).";
				return result;
			}
			for(char const& c : s[1])
			{
				if(c < '0' || c > '9')
				{
					subargs.subcommand = arg::GenerateSubCommand::INVALID;
					subargs.errorMessage = "Invalid max particles per node (not an integer number): '" + s[1] + "'";
					return result;
				}
			}
			subargs.outputOptions.maxParticlesPerNode = atoi(s[1].c_str());
		}
	}
	// Output
	if(subargs.output.empty())
	{
		subargs.subcommand = arg::GenerateSubCommand::INVALID;
		subargs.errorMessage = "No output specified.";
		return result;
	}

	return result;
}

arg::Arguments handle_arguments(int argc, char* argv[])
{
	if(argc <= 1)
	{
		return arg::Arguments(arg::Command::HELP);
	}

	std::string command(argv[1]);
	if(command == "-h" || command == "--help")
	{
		return arg::Arguments(arg::Command::HELP);
	}
	if(command == "info")
	{
		std::vector<std::string> remainingArgs;
		for(int i(0); i < argc - 2; ++i)
		{
			remainingArgs.push_back(argv[2 + i]);
		}
		return handle_info_arguments(remainingArgs);
	}
	if(command == "generate")
	{
		std::vector<std::string> remainingArgs;
		for(int i(0); i < argc - 2; ++i)
		{
			remainingArgs.push_back(argv[2 + i]);
		}
		return handle_generate_arguments(remainingArgs);
	}

	arg::Arguments result(arg::Command::INVALID);
	auto& args(*static_cast<arg::InvalidArguments*>(result.subargs));
	args.wrongCommand = command;
	return result;
}
