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

#ifndef FILE_HANDLE_FACTORY_H_
#define FILE_HANDLE_FACTORY_H_

#include <map>
#include <boost/weak_ptr.hpp>

#include <EventSystem/Emitter.h>

#include <Core/XmlFileHandleFactory.h>
//#include <Model/Property/Plugin.h>

namespace BFG {

//! This class provides the creation of filehanling objects of a specific type.
//! ConfigT Type of config data object
template<class FileHandlerT>
class FileHandleFactory
{

public:

typedef typename FileHandlerT::ReturnT ConfigT;

FileHandleFactory(const std::vector<std::string>& files)
{
	std::vector<std::string>::const_iterator it = files.begin();
	XmlFileHandleT fileHandle;
	
	for (;it != files.end(); ++it)
	{
		fileHandle = createXmlFileHandle(*it);
		mFileHandlerList.push_back(FileHandlerT_ptr(new FileHandlerT(fileHandle)));
	}
}

ConfigT requestConfig(const std::string& configName) const
{
	ConfigT result;
	typename FileHandlerListT::const_iterator it = mFileHandlerList.begin();
	
	for (; it != mFileHandlerList.end(); ++it)
	{
		FileHandlerT_ptr fileHandler = *it;
		result = fileHandler->create(configName);
		
		if (result)
			return result;
	}

	// construct error message.
	std::vector<std::string>::const_iterator fileIt = mFileList.begin();
	std::string files = "FileList ";
	
	for(;fileIt != mFileList.end(); ++fileIt)
		files += *fileIt+" ";
	
	throw std::logic_error("Could not find: "+configName+" in: "+files+".");
}

private:

typedef boost::shared_ptr<FileHandlerT> FileHandlerT_ptr;
typedef std::vector<FileHandlerT_ptr> FileHandlerListT;

FileHandlerListT mFileHandlerList;
std::vector<std::string> mFileList;

};

} // namespace BFG

#endif
