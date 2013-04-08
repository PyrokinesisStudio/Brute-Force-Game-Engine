/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2012 Brute-Force Games GbR

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

#include <Core/PugiXmlFileHandle.h>

#ifdef BFG_USE_PUGIXML

#include <Base/Logger.h>
#include <Core/PugiXmlTree.h>

namespace BFG {

PugiXmlFileHandle::PugiXmlFileHandle(const std::string& path, bool createFile): XmlFileHandle(path)
{
	try
	{
		if (createFile)
		{
			save();
		}
		else
		{
			pugi::xml_parse_result result = mDocument.load_file(mPath.c_str());
		
			if (result.status != pugi::status_ok)
			{
				switch (result)
				{
				case pugi::status_file_not_found:
						throw std::logic_error("Error during xml file parsing. File not found: "+path);
				default:
					throw std::logic_error("Error during xml file parsing: "+path+" "+result.description());
				}
			}
		}

	}
	catch (std::exception& e)
	{
		throw std::logic_error("Error during xml file parsing: "+path+". At PugiXmlFileHandle::PugiXmlFileHandle. "+e.what());
	}

	mRoot.reset(new PugiXmlTree(mDocument));

	dbglog << "Parsed: " << path << ".";
}


void PugiXmlFileHandle::save(const std::string& path)
{
	std::string errorInformationPath = "";

	try
	{
		if (path == "")
		{
			errorInformationPath = mPath;
			dbglog << mDocument.save_file(mPath.c_str());
		}
		else
		{
			errorInformationPath = path;
			dbglog << mDocument.save_file(path.c_str());
		}
	}
	catch (std::exception& e)
	{
		throw std::logic_error("Error during attempt saving xml file "+errorInformationPath+". At PugiXmlFileHandle::save. "+e.what());
	}
}

} // namespace BFG

#endif