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

#include <fstream>
#include <future>
#include <iostream>
#include <vector>

#include <liboctree/Octree.hpp>
#include <liboctree/binaryrw.hpp>

#include "utils.hpp"
#include "handle_arguments.hpp"

void executeHelp(std::string const& argv_0)
{
	std::cout << "Usage: " << std::endl
	          << "\t" << argv_0 << " [-h|--help]" << std::endl
			  << "\t\t Prints this help message." << std::endl
	          << "\t" << argv_0 << " <COMMAND> [COMMAND-OPTIONS]" << std::endl << std::endl
	          << "\tCommands:" << std::endl
	          << "\t\t" << argv_0 << " info ..." << std::endl
	          << "\t\t\tPrints informations from an existing octree file." << std::endl
	          << "\t\t" << argv_0 << " generate..." << std::endl
	          << "\t\t\tGenerates an octree file given data from various sources." << std::endl << std::endl
	          << "\t\tAll commands have a [-h|-help] option to display their own help page." << std::endl;
}

void executeInvalid(arg::InvalidArguments const& args, std::string const& argv_0)
{
	std::cerr << "ERROR: Invalid command \"" << args.wrongCommand << "\"." << std::endl;
	executeHelp(argv_0);
}

void executeInfoHelp(std::string const& argv_0)
{
	std::cout << "Usage: " << std::endl
	          << "\t" << argv_0 << " info [-h|--help]" << std::endl
			  << "\t\t Prints this help message." << std::endl
	          << "\t" << argv_0 << " info <OCTREE-FILE>" << std::endl
			  << "\t\t Prints informations about OCTREE_FILE. Cannot handle globbing or several files as an input." << std::endl;
}

void executeInfo(arg::InfoArguments const& args, std::string const& argv_0)
{
	switch(args.subcommand)
	{
		case arg::InfoSubCommand::INVALID:
		std::cerr << "ERROR: Invalid info command: " << args.errorMessage << std::endl;
		executeInfoHelp(argv_0);
		return;
		case arg::InfoSubCommand::HELP:
		executeInfoHelp(argv_0);
		return;
		case arg::InfoSubCommand::INFO:
		break;
	}

	Octree octree;
	readOctreeStructureOnly(args.input, octree);

	std::cout << args.input << " :" << std::endl;
	std::cout << '\t' << "Bounding box :" << std::endl;
	std::cout << "\t\t" << "x:[" << octree.getMinX() << ',' << octree.getMaxX() << ']' << std::endl;
	std::cout << "\t\t" << "y:[" << octree.getMinY() << ',' << octree.getMaxY() << ']' << std::endl;
	std::cout << "\t\t" << "z:[" << octree.getMinZ() << ',' << octree.getMaxZ() << ']' << std::endl;
	std::cout << '\t' << "Flags :";

	std::string flagsStr("  ");

	auto flags = octree.getFlags();
	if((flags & Octree::Flags::NORMALIZED_NODES) != Octree::Flags::NONE)
	{
		flagsStr += "NORMALIZED_NODES, ";
	}
	if((flags & Octree::Flags::VERSIONED) != Octree::Flags::NONE)
	{
		flagsStr += "VERSIONED, ";
	}
	if((flags & Octree::Flags::STORE_RADIUS) != Octree::Flags::NONE)
	{
		flagsStr += "STORE_RADIUS, ";
	}
	if((flags & Octree::Flags::STORE_LUMINOSITY) != Octree::Flags::NONE)
	{
		flagsStr += "STORE_LUMINOSITY, ";
	}
	if((flags & Octree::Flags::STORE_COLOR) != Octree::Flags::NONE)
	{
		flagsStr += "STORE_COLOR, ";
	}
	if((flags & Octree::Flags::STORE_DENSITY) != Octree::Flags::NONE)
	{
		flagsStr += "STORE_DENSITY, ";
	}
	if((flags & Octree::Flags::STORE_TEMPERATURE) != Octree::Flags::NONE)
	{
		flagsStr += "STORE_TEMPERATURE, ";
	}
	// remote last two characters
	flagsStr.resize(flagsStr.size() - 2);
	std::cout << flagsStr << std::endl;
	std::cout << '\t' << "Vertex dimension : " << octree.getDimPerVertex() << std::endl;

	std::ifstream in;
	in.open(args.input, std::fstream::in | std::fstream::binary);
	octree.readOwnData(in);
	auto data(octree.getOwnData());
	if(data.empty())
	{
		std::cout << "Empty data..." << std::endl;
		return;
	}
	std::cout << "\tNumber of vertices : " << octree.getTotalDataSize() / octree.getDimPerVertex() << std::endl;
	unsigned int i(0);
	std::cout << "\tData sample :" << std::endl << "\t\t";
	for(float d : data)
	{
		std::cout << d << ", ";
		++i;
		if(i / octree.getDimPerVertex() > 15)
		{
			std::cout << std::endl;
			break;
		}
		if(i % octree.getDimPerVertex() == 0)
		{
			std::cout << std::endl;
			if(i < data.size() - 1)
			{
				std::cout << "\t\t";
			}
		}
	}
}

void executeGenerateHelp(std::string const& argv_0)
{
	std::cout << "Usage: " << std::endl
	          << "\t" << argv_0 << " generate [-h|--help]" << std::endl
			  << "\t\t Prints this help message." << std::endl
	          << "\t" << argv_0 << " generate [INPUT-OPTIONS] <INPUT> --output [OUTPUT-OPTIONS] <OCTREE-FILE-OUT>" << std::endl
			  << "\t\tTakes some input data and generates an octree written in OCTREE-FILE-OUT." << std::endl << std::endl 
			  << "\tINPUT OPTIONS:" << std::endl << "\t\t--sample-rate=<RATE> : resamples the input to only take RATE fraction particles (ex: --sample-rate=0.5 halves the input data)." << std::endl << std::endl

		<< "\tINPUT:" << std::endl
	<< "\t\tEither of:" << std::endl
	<< "\t\t--input-random <PARTICLES-NUMBER> [ADDITIONAL-DIMENSIONS] : specifies random data as particles, generates PARTICLES-NUMBER particles. Additional random dimensions can be specified as ADDITIONAL-DIMENSIONS :" << std::endl
		<< "\t\t\t--add-radius" << std::endl
		<< "\t\t\t--add-lum" << std::endl
		<< "\t\t\t--add-rgb-lum" << std::endl
		<< "\t\t\t--add-density" << std::endl
		<< "\t\t\t--add-temperature" << std::endl
	<< "\t\t--input-octree <OCTREE-FILES> : specifies octree file(s) as input (globbing works). If several files are specified, they must share the same flags (check flags using octreegen info)." << std::endl
    << "\t\t--input-hdf5 <HDF5-FILES> --coord-path=<COORD-DATASET-PATH> [ADDITIONAL-DATASET-PATHS] : specifies hdf5 file(s) as input (glob works). If several files are specified, they must share the same dataset path structure. COORD-DATASET-PATH is the 3D dataset path of particles coordinates. Additional variables dataset path can be specified as ADDITIONAL-DATASET-PATHS :" << std::endl
        << "\t\t\t--radius-path=<RADIUS-DATASET-PATH> : 1D dataset" << std::endl
        << "\t\t\t--lum-path=<LUM-DATASET-PATH> : total luminosity (1D dataset)" << std::endl
        << "\t\t\t--rgb-lum-path=<RGB-LUM-PATH> : luminosity per band (3D dataset)" << std::endl
        << "\t\t\t--density-path=<DENSITY-DATASET-PATH> : 1D dataset" << std::endl
        << "\t\t\t--temperature-path=<TEMPERATURE-DATASET-PATH> : 1D dataset" << std::endl << std::endl

	<< "\tOUTPUT-OPTIONS" << std::endl
    << "\t\t--disable-node-normalization : disables particles having coordinates in [0;1] relative to their node, which is on by default" << std::endl
	<< "\t\t--max-particles-per-node=<MAX_PART_PER_NODE> : defines a particle number above which a node is split in 8 sub-nodes (and below which it becomes a leaf). MAX_PART_PER_NODE is 16000 by default." << std::endl << std::endl;

	std::cout << "Examples: " << std::endl
	          << "\t"
	          << "To read gaz data coordinates and luminosity within snapshot.*.hdf5 files (will be expanded as \"snapshot.0.hdf5 snapshot.1.hdf5\" for example) in group /PartType0 and write the corresponding octree in the gaz.octree file :" << std::endl
	          << "\t" << argv_0
	          << " generate --input-hdf5 snapshot.*.hdf5 --coord-path=/PartType0/Coordinates --lum-path=/PartType0/Luminosities --output gaz.octree" << std::endl
			  << "\twhich is equivalent to :" << std::endl
	          << "\t" << argv_0
	          << " generate --input-hdf5 snapshot.0.hdf5 snapshot.1.hdf5 --coord-path=/PartType0/Coordinates --lum-path=/PartType0/Luminosities --output gaz.octree" << std::endl
	          << std::endl
	          << "\t"
	          << "To generate 1 million uniformly random particles and "
	          << std::endl
	          << "\twrite the corresponding octree in the random.octree file "
	             ":"
	          << std::endl
	          << "\t" << argv_0 << " --input-random 1000000 --output random.octree" << std::endl;
}

void executeGenerate(arg::GenerateArguments const& args, std::string const& argv_0)
{
	switch(args.subcommand)
	{
		case arg::GenerateSubCommand::INVALID:
		std::cerr << "ERROR: Invalid generate command: " << args.errorMessage << std::endl;
		executeGenerateHelp(argv_0);
		return;
		case arg::GenerateSubCommand::HELP:
		executeGenerateHelp(argv_0);
		return;
		case arg::GenerateSubCommand::GENERATE:
		break;
	}
	// Debug message
	std::cout << "Generate :" << std::endl;
	std::cout << "Input options :" << std::endl;
	std::cout << "\tSample rate :\t\t\t" << args.inputOptions.sampleRate << std::endl;
	std::cout << std::endl;

	switch(args.inputType)
	{
		case arg::GenerateInputType::RANDOM:
			std::cout << "Input Type :\t\t\t\tRANDOM" << std::endl;
			std::cout << "\tParticles number :\t\t" << args.randomInputArgs.particlesNumber << std::endl;
			std::cout << "\tAdd radius :\t\t\t" << (args.randomInputArgs.radius ? "on" : "off") << std::endl;
			std::cout << "\tAdd lum :\t\t\t" << (args.randomInputArgs.lum ? "on" : "off") << std::endl;
			std::cout << "\tAdd RGB lum :\t\t\t" << (args.randomInputArgs.rgbLum ? "on" : "off") << std::endl;
			std::cout << "\tAdd density :\t\t\t" << (args.randomInputArgs.density ? "on" : "off") << std::endl;
			std::cout << "\tAdd temperature :\t\t" << (args.randomInputArgs.temperature ? "on" : "off") << std::endl;
			break;
		case arg::GenerateInputType::OCTREE:
			std::cout << "Input Type :\t\t\t\tOCTREE" << std::endl;
			std::cout << "\tFiles :" << std::endl;
			for(auto const& f : args.octreeInputArgs.octreeFiles)
			{
				std::cout << "\t\t" << f << std::endl;
			}
			break;
		case arg::GenerateInputType::HDF5:
			std::cout << "Input Type :\t\t\t\tHDF5" << std::endl;
			std::cout << "\tFiles :" << std::endl;
			for(auto const& f : args.hdf5InputArgs.hdf5Files)
			{
				std::cout << "\t\t" << f << std::endl;
			}
			std::cout << "\tCoord path :\t\t\t'" << args.hdf5InputArgs.coordPath << "'" << std::endl;
			std::cout << "\tRadius path :\t\t\t'" << args.hdf5InputArgs.radiusPath << "'" << std::endl;
			std::cout << "\tLum path :\t\t\t'" << args.hdf5InputArgs.lumPath << "'" << std::endl;
			std::cout << "\tRGB lum path :\t\t\t'" << args.hdf5InputArgs.rgbLumPath << "'" << std::endl;
			std::cout << "\tDensity path :\t\t\t'" << args.hdf5InputArgs.densityPath << "'" << std::endl;
			std::cout << "\tTemperature path :\t\t'" << args.hdf5InputArgs.temperaturePath << "'" << std::endl;
			break;
		default:
			std::cout << "Input Type : INVALID" << std::endl;
			break;
	}
	std::cout << std::endl;

	std::cout << "Output options :" << std::endl;
	std::cout << "\tNode normalization :\t\t" << (args.outputOptions.normalizeNodes ? "on" : "off") << std::endl;
	std::cout << "\tMax particles per node :\t" << args.outputOptions.maxParticlesPerNode << std::endl;
	std::cout << std::endl;

	std::cout << "Output :\t\t\t\t" << args.output << std::endl << std::endl;


	std::vector<float> v; // contains octree construction data
	Octree::Flags flags(Octree::Flags::NONE);
	// CONSTRUCT INPUT
	switch(args.inputType)
	{
		case arg::GenerateInputType::RANDOM:
			{
				size_t numberOfVertices(args.randomInputArgs.particlesNumber * args.inputOptions.sampleRate);
				unsigned int dimPerVertex(3);
				if(args.randomInputArgs.radius)
				{
					flags |= Octree::Flags::STORE_RADIUS;
					dimPerVertex += 1;
				}
				if(args.randomInputArgs.lum)
				{
					flags |= Octree::Flags::STORE_LUMINOSITY;
					dimPerVertex += 1;
				}
				if(args.randomInputArgs.rgbLum)
				{
					flags |= Octree::Flags::STORE_COLOR;
					dimPerVertex += 3;
				}
				if(args.randomInputArgs.density)
				{
					flags |= Octree::Flags::STORE_DENSITY;
					dimPerVertex += 1;
				}
				if(args.randomInputArgs.temperature)
				{
					flags |= Octree::Flags::STORE_TEMPERATURE;
					dimPerVertex += 1;
				}
				v = generateVertices(numberOfVertices, time(NULL), dimPerVertex);
			}
			break;
		case arg::GenerateInputType::OCTREE:
			for(auto const& f : args.octreeInputArgs.octreeFiles)
			{
				std::cout << "Loading '" << f << "'..." << std::endl;
				Octree oc;
				readOctreeStructureOnly(f, oc);
				if(flags == Octree::Flags::NONE)
				{
					flags = oc.getFlags() & ~Octree::Flags::VERSIONED & ~Octree::Flags::NORMALIZED_NODES;
				}
				else if((oc.getFlags() & ~Octree::Flags::VERSIONED & ~Octree::Flags::NORMALIZED_NODES) != flags)
				{
					std::cerr << "ERROR: Octree files don't share the same flags (VERSIONED and NORMALIZED_NODES don't count)." << std::endl;
					return;
				}
				readOctreeContentOnly(f, oc);

				std::vector<float> data;
				std::cout << "Extracting data :" << std::endl;
				Octree::showProgress(0.f);
				oc.dumpInVectorAndEmpty(data);
				Octree::showProgress(1.f);
				v.reserve(v.size() + data.size() * args.inputOptions.sampleRate);
				size_t addedVertices(data.size() / oc.getDimPerVertex());
				if(args.inputOptions.sampleRate >= 1.f)
				{
					v.insert(v.end(), data.begin(), data.end());
				}
				else
				{
					addedVertices = 0;
					std::cout << "Subsampling data :" << rand() << std::endl;
					Octree::showProgress(0.f);
					for(size_t i(0); i < data.size(); i += oc.getDimPerVertex())
					{
						if((static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
							  < args.inputOptions.sampleRate)
						{
							++addedVertices;
							for(size_t j(0); j < oc.getDimPerVertex(); ++j)
							{
								v.push_back(data[i+j]);
							}
						}
						if(i % 10000000 == 0)
						{
							Octree::showProgress(static_cast<float>(i) / data.size());
						}
					}
					Octree::showProgress(1.f);
				}
				std::cout << "Added " << addedVertices << " vertices." << std::endl;
			}
			break;
		case arg::GenerateInputType::HDF5:
			try
			{
				size_t verticesNumber = totalNumberOfVertices(args.hdf5InputArgs.hdf5Files, args.hdf5InputArgs.coordPath.c_str());
				std::cout << "Total number of vertices to read : " << verticesNumber << std::endl;

				unsigned int stride(3);
				size_t fileOffset(0), offset(0);
				if(!args.hdf5InputArgs.radiusPath.empty())
				{
					flags |= Octree::Flags::STORE_RADIUS;
					stride++;
				}
				if(!args.hdf5InputArgs.lumPath.empty())
				{
					flags |= Octree::Flags::STORE_LUMINOSITY;
					stride++;
				}
				if(!args.hdf5InputArgs.rgbLumPath.empty())
				{
					flags |= Octree::Flags::STORE_COLOR;
					stride += 3;
				}
				if(!args.hdf5InputArgs.densityPath.empty())
				{
					flags |= Octree::Flags::STORE_DENSITY;
					stride++;
				}
				if(!args.hdf5InputArgs.temperaturePath.empty())
				{
					flags |= Octree::Flags::STORE_TEMPERATURE;
					stride++;
				}
				for(auto file : args.hdf5InputArgs.hdf5Files)
				{
					std::vector<float> data;
					std::cout << file << std::endl;

					offset = 0;
					size_t fileOffset_back(fileOffset);

					Octree::showProgress(0.f);
					fileOffset += stride
								  * readHDF5Dataset(file, args.hdf5InputArgs.coordPath.c_str(), data,
													fileOffset_back + offset, stride);
					offset += 3;
					Octree::showProgress(offset / (float) stride);
					if(!args.hdf5InputArgs.radiusPath.empty())
					{
						readHDF5Dataset(file, args.hdf5InputArgs.radiusPath.c_str(), data,
										fileOffset_back + offset, stride);
						offset++;
						Octree::showProgress(offset / (float) stride);
					}
					if(!args.hdf5InputArgs.lumPath.empty())
					{
						readHDF5Dataset(file, args.hdf5InputArgs.lumPath.c_str(), data,
										fileOffset_back + offset, stride);
						offset++;
						Octree::showProgress(offset / (float) stride);
					}
					if(!args.hdf5InputArgs.rgbLumPath.empty())
					{
						readHDF5Dataset(file, args.hdf5InputArgs.rgbLumPath.c_str(), data,
										fileOffset_back + offset, stride);
						offset += 3;
						Octree::showProgress(offset / (float) stride);
					}
					if(!args.hdf5InputArgs.densityPath.empty())
					{
						readHDF5Dataset(file, args.hdf5InputArgs.densityPath.c_str(), data,
										fileOffset_back + offset, stride);
						offset++;
						Octree::showProgress(offset / (float) stride);
					}
					if(!args.hdf5InputArgs.temperaturePath.empty())
					{
						readHDF5Dataset(file, args.hdf5InputArgs.temperaturePath.c_str(), data,
										fileOffset_back + offset, stride);
						offset++;
						Octree::showProgress(offset / (float) stride);
					}
					v.reserve(v.size() + data.size() * args.inputOptions.sampleRate);
					size_t addedVertices(data.size() / stride);
					if(args.inputOptions.sampleRate >= 1.f)
					{
						v.insert(v.end(), data.begin(), data.end());
					}
					else
					{
						addedVertices = 0;
						std::cout << "Subsampling data :" << std::endl;
						Octree::showProgress(0.f);
						for(size_t i(0); i < data.size(); i += stride)
						{
							if((static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
								  < args.inputOptions.sampleRate)
							{
								++addedVertices;
								for(size_t j(0); j < stride; ++j)
								{
									v.push_back(data[i+j]);
								}
							}
							if(i % 10000000 == 0)
							{
								Octree::showProgress(static_cast<float>(i) / data.size());
							}
						}
						Octree::showProgress(1.f);
					}
					std::cout << "Added " << addedVertices << " vertices." << std::endl;
				}

				std::cout << "Loaded from file(s) : " << v.size() / stride << " points"
						  << std::endl;
			}
			catch(std::string s)
			{
				std::cerr << "Error while reading HDF5 file(s) :" << std::endl;
				std::cerr << s << std::endl;
				return;
			}
			break;
		default:
			std::cerr << "ERROR: Invalid generate command: unknown input type." << std::endl;
			executeGenerateHelp(argv_0);
			return;
	}
	// OUTPUT
	if(args.outputOptions.normalizeNodes)
	{
		flags |= Octree::Flags::NORMALIZED_NODES;
	}

	Octree octree;

	octree.setFlags(flags);
	std::cout << "Constructing octree :" << std::endl;
	Octree::showProgress(0.f);
	octree.init(v, args.outputOptions.maxParticlesPerNode);

	std::ofstream f(args.output, std::ios_base::out | std::ios_base::binary);
	std::cout << "Writing octree to output file '" << args.output << "' :" << std::endl;
	Octree::showProgress(0.f);
	write(f, octree);
	Octree::showProgress(1.f);
	f.close();
	std::cout << "Conversion successfull !" << std::endl;
}

int main(int argc, char* argv[])
{
	auto arguments(handle_arguments(argc, argv));
	switch(arguments.command)
	{
		case arg::Command::INVALID:
			executeInvalid(*static_cast<arg::InvalidArguments*>(arguments.subargs), argv[0]);
			break;
		case arg::Command::HELP:
			executeHelp(argv[0]);
			break;
		case arg::Command::INFO:
			executeInfo(*static_cast<arg::InfoArguments*>(arguments.subargs), argv[0]);
			break;
		case arg::Command::GENERATE:
			executeGenerate(*static_cast<arg::GenerateArguments*>(arguments.subargs), argv[0]);
			break;
	}
	return EXIT_SUCCESS;
}
