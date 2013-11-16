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

#ifndef BFG_XML_FILE_HANDLE__
#define BFG_XML_FILE_HANDLE__

#include <boost/shared_ptr.hpp>
#include <Core/XmlTree.h>
#include <Core/Defs.h>


namespace BFG {

class XmlFileHandle;

typedef boost::shared_ptr<XmlFileHandle> XmlFileHandleT;

class BFG_CORE_API XmlFileHandle
{

public:

	XmlFileHandle(const std::string& path): mPath(path) {}

	virtual void save(const std::string& path = "") = 0;
	
	//! Root is not(!) the first element in the XML file. To get the first element call root()->child("FirstElementName") .
	XmlTreeT root() { return mRoot; }
	std::string path() { return mPath; }

protected:

	std::string mPath;
	XmlTreeT mRoot;
};

} // namespace BFG

#endif
