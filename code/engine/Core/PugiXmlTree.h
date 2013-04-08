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


#ifndef BFG_PUGI_XML_TREE__
#define BFG_PUGI_XML_TREE__

#include <Core/Defs.h>

#ifdef BFG_USE_PUGIXML

#include <Core/XmlTree.h>
#include <pugixml.hpp>

namespace BFG {

class BFG_CORE_API PugiXmlTree : public XmlTree
{

	friend class PugiXmlFileHandle;

public:

	XmlTreeT child(const std::string& name) const;

	XmlTreeListT childList(const std::string& name) const;
	std::string attribute(const std::string& name) const;

    //! \return Returns an empty string if element has no data.
    std::string elementData() const;
	void editElementData(const std::string& value);
	void editAttributeData(const std::string& name, const std::string& value);

	XmlTreeT addElement(const std::string& name);
	XmlTreeT addElement(const std::string& name, const std::string& value);
	void addAttribute(const std::string& name, const std::string& value);

	void removeElement(const std::string& name);
	void removeAttribute(const std::string& name);

private:

	PugiXmlTree(pugi::xml_node& node): mXmlNode(node) {}
	PugiXmlTree(const PugiXmlTree& copy)
	{
		mXmlNode = copy.mXmlNode;
	}

	pugi::xml_node mXmlNode;
};

} // namespace BFG

#endif
#endif
