/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>

#include <boost/filesystem/config.hpp>  // for filesystem
#include <boost/filesystem/fstream.hpp>  // for ofstream
#include <boost/filesystem/operations.hpp>  // for exists, last_write_time, etc
#include <boost/filesystem/path.hpp>    // for path, native

#include <boost/program_options.hpp>

#include <boost/array.hpp>
#include <boost/crc.hpp>

#if defined(__linux) || defined(linux)
#include <errno.h> // program_invocation_name
#endif

#include "Parser.h"
#include "Generator.h"

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;

static const string InputString = "input";
static const string HeaderString = "header";
static const string SourceString = "source";
static const string HelpString = "help";
static const string ForceString = "force";

using namespace EnumGen;


std::string calculateFileCRC(const std::string& Filename)
{
	boost::crc_32_type result;
	std::string        result_str;
	
	std::ifstream file(Filename.c_str(), ios::binary);
	
	while (file)
	{
		const size_t buffersize = 4096;
		boost::array<char, buffersize> buffer;
		file.read(buffer.data(), buffersize);
		result.process_bytes(buffer.data(), file.gcount());
	}
	file.close();

	std::stringstream ss;
	ss << std::hex << std::uppercase << result.checksum();
	result_str = ss.str();
	
	if (result_str.empty())
		throw std::runtime_error
			("calculateFileCRC: Unable to calculate checksum");
	
	return result_str;
}

Shared::EnumVector loadFile(const std::string& Filename)
{
	TiXmlDocument doc(Filename);
	bool OK = doc.LoadFile();

	if (! OK)
		throw Exception::FileNotFound(Filename);

	TiXmlHandle hDoc(&doc);
	TiXmlHandle hRoot(0);

	hRoot = Parse::Header(hDoc);

	Shared::EnumVector Everything;

	Parse::Content(hRoot, Everything);

	return Everything;
}

void showUsage(po::options_description desc)
{
	// +Show BFG-Header
	cout << "Usage ...." << endl << endl
		 << desc << "\n";
}

void checkAndPrintTimeTravelWarning(const std::string& filenameWithPath)
{
	time_t lastWriteTime = fs::last_write_time(filenameWithPath);
	time_t now = time(NULL);

	time_t difftime = now - lastWriteTime;
	if (difftime < 0)
	{
		cout << "Warning: Modification time of \"" << filenameWithPath
		     << "\" is "
		     << difftime
		     << " seconds in future "
		     << endl;
	}
}

bool firstFileIsNewer(const std::string& first, const std::string& second)
{
	time_t firstLastWriteTime = fs::last_write_time(first);
	time_t secondLastWriteTime = fs::last_write_time(second);
	
	return firstLastWriteTime > secondLastWriteTime;
}

bool mustGenerate(const std::string& inputFile,
                  const std::string& headerFile,
                  const std::string& sourceFile,
                  const std::string& enumgenFile,
                  bool forceGeneration)
{
	if (forceGeneration)
		return true;

	// Generate, if we can't determine if the enumgen binary is newer or not.
	if (!(fs::exists(enumgenFile) &&
	      fs::is_regular_file(enumgenFile)))
	{
		cout << "Warning, unable to test if "
		        "EnumGen itself is newer than the "
		        "output file. Got: \""
		     << enumgenFile
		     << "\" instead of "
		        "EnumGen.exe" << endl;
		return true;
	}

	// .hh empty or not existant?
	if (fs::file_size(headerFile) == 0 ||
	    !fs::exists(headerFile))
		return true;

	// .hh older than .xml or enumgen binary?
	if (firstFileIsNewer(inputFile, headerFile) ||
	    firstFileIsNewer(enumgenFile, headerFile))
		return true;

	if (! sourceFile.empty())
	{
		// .cpp empty or not existant?
		if (fs::file_size(sourceFile) == 0 ||
		    !fs::exists(sourceFile))
			return true;

		// .cpp older than .xml or enumgen binary?
		if (firstFileIsNewer(inputFile, sourceFile) ||
		    firstFileIsNewer(enumgenFile, sourceFile))
			return true;
	}
	
	return false;
}

int main(int argc, char* argv[]) try
{
#if defined (_WIN32)
	std::string RunCommand(argv[0]);
#elif defined (linux) || defined (__linux) || (_POSIX_VERSION >= 200112L)
	std::string RunCommand(program_invocation_name);
#else
  #error Implement this Platform
#endif
	assert(! RunCommand.empty());
	fs::path full_path(fs::initial_path());
	full_path = fs::system_complete(fs::path(RunCommand));
	std::string PathEnumGenExe = full_path.string();

#if defined (_WIN32)
	assert(PathEnumGenExe.length() > 4);
	if (PathEnumGenExe.substr(PathEnumGenExe.length()-4, 4) != ".exe")
		PathEnumGenExe.append(".exe");
#endif


	po::options_description desc("Allowed options");
	desc.add_options()
		(HelpString.c_str(),
		"Display this help message")

		(InputString.c_str(), po::value<std::string>(),
		"Set .xml file. This is the file which gets parsed by EnumGen.")

		(HeaderString.c_str(), po::value<std::string>(),
		"Set .hh file. File name of the header which gets generated.")
		
		(SourceString.c_str(), po::value<std::string>(),
		"Set .cpp file. File name of the source which gets generated.\n"
		"If you omit this parameter, everything will be put into the header.")
		
		(ForceString.c_str(),
		"Ignore time checks and force overwrite")
        ;

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);    

	if (vm.count(HelpString)) 
	{
		showUsage(desc);
		return 1;
	}

	if (vm.count(InputString) && vm.count(HeaderString)) 
	{
		bool forceGeneration = vm.count(ForceString) != 0;
		string inputFile  = vm[InputString].as<std::string>();
		string headerFile = vm[HeaderString].as<std::string>();
		string sourceFile;
		
		if (vm.count(SourceString) != 0)
			sourceFile = vm[SourceString].as<std::string>();

		if (!fs::exists(inputFile))
		{
			cout << "InputFile \"" << inputFile << "\" does not exist!" << endl;
			return 1;
		}

		if (0 == fs::file_size(inputFile))
		{
			cout << "InputFile is empty!" << endl;
			return 1;
		}

		checkAndPrintTimeTravelWarning(inputFile);

		bool generationNecessary = mustGenerate
		(
			inputFile,
			headerFile,
			sourceFile,
			PathEnumGenExe,
			forceGeneration
		);

		if (!generationNecessary)
		{
			cout << "Target file is up-to-date" << endl
			     << "Generation stopped" << endl;
			return 0;
		}
		
		Shared::EnumVector inputEnums = loadFile(inputFile);

		if (sourceFile.empty())
		{
			std::string Result;
			std::string inputFileCRC = calculateFileCRC(inputFile);			

			Generator::MakeHeaderOnly
			(
				Result,
				inputFileCRC,
				inputEnums
			);

			fs::ofstream out_file(headerFile, std::ios_base::out);
			if (!out_file)
				throw std::runtime_error("Could not open OutputFile!");

			out_file.write(Result.c_str(), Result.size());

			cout << "Successfully compiled "
			     << inputFile << " to:\n"
			     << "-> " << headerFile << endl;
		}
		else
		{
			std::string headerResult;
			std::string sourceResult;
			std::string inputFileCRC = calculateFileCRC(inputFile);

			size_t pos = headerFile.find_last_of("/\\");
			std::string HeaderFileNameWithoutPath(headerFile.substr(pos + 1));

			Generator::MakePair
			(
				headerResult,
				sourceResult,
				HeaderFileNameWithoutPath,
				inputFileCRC,
				inputEnums
			);

			fs::ofstream hf(headerFile, std::ios_base::out);
			fs::ofstream sf(sourceFile, std::ios_base::out);

			if (! hf)
				throw std::runtime_error("Unable to open header file for write!");

			if (! sf)
				throw std::runtime_error("Unable to open source file for write!");

			hf.write(headerResult.c_str(), headerResult.size());
			sf.write(sourceResult.c_str(), sourceResult.size());

			cout << "Successfully compiled "
			     << inputFile << " to:\n"
			     << "-> " << headerFile << " and\n"
			     << "-> " << sourceFile << endl;
		}
	}
	else
	{
		cout << "input/output file was not set.\n";
		showUsage(desc);
	}
}
catch (std::exception& ex)
{
	cerr << "EnumGen Exception: " << ex.what() << endl;
	return 1;
}
