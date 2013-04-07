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

#ifndef BFG_PUGI_XML_FILE_HANDLE__
#define BFG_PUGI_XML_FILE_HANDLE__

#include <Core/Defs.h>

#ifdef BFG_USE_PUGIXML

#include <boost/shared_ptr.hpp>
#include <pugixml.hpp>
#include <Core/XmlFileHandle.h>

namespace BFG {

class BFG_CORE_API PugiXmlFileHandle : public XmlFileHandle
{

public:

	PugiXmlFileHandle(const std::string& path, bool createFile = false);
	void save(const std::string& path = "");

private:

	pugi::xml_document mDocument;
};

} // namespace BFG

#endif
#endif
