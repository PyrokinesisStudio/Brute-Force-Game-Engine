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

#include <Core/PugiXmlTree.h>

#ifdef BFG_USE_PUGIXML

#include <stdexcept>


namespace BFG {

	XmlTreeT PugiXmlTree::child(const std::string& name) const
	{
		pugi::xml_node node = mXmlNode.child(name.c_str());
		return XmlTreeT(new PugiXmlTree(node));
	}


	XmlTreeListT PugiXmlTree::childList(const std::string& name) const
	{
		XmlTreeListT treeList;

		for (pugi::xml_node node = mXmlNode.child(name.c_str()); node; node = node.next_sibling(name.c_str()))
		{
			treeList.push_back(XmlTreeT(new PugiXmlTree(node)));
		}

		return treeList;
	}


	std::string PugiXmlTree::attribute(const std::string& name) const
	{
		return mXmlNode.attribute(name.c_str()).value();
	}


    std::string PugiXmlTree::elementData() const
    {
        return mXmlNode.text().get();
    }


	void PugiXmlTree::editElementData(const std::string& value)
	{
		mXmlNode.text().set(value.c_str());
	}


    void PugiXmlTree::editAttributeData(const std::string& name, const std::string& value)
	{
		pugi::xml_attribute attribute = mXmlNode.attribute(name.c_str());
		
		if (attribute)
			attribute.set_value(value.c_str());
		else
			throw std::logic_error("No attribute found with name: "+name);
	}
	
	
	XmlTreeT PugiXmlTree::addElement(const std::string& name, const std::string& value)
	{
		XmlTreeT node = addElement(name);
		node->editElementData(value);
		
		return node;
	}


	XmlTreeT PugiXmlTree::addElement(const std::string& name)
	{
		pugi::xml_node node = mXmlNode.append_child();
		
		if (!node.set_name(name.c_str()))
		{
			throw std::logic_error("Could not set name for new XML Node. At PugiXmlTree::addElement(...)");
		}

		return XmlTreeT(new PugiXmlTree(node));
	}


	void PugiXmlTree::addAttribute(const std::string& name, const std::string& value)
	{
		mXmlNode.append_attribute(name.c_str()).set_value(value.c_str());
	}


	void PugiXmlTree::removeElement(const std::string& name)
	{
		if (!mXmlNode.remove_child(name.c_str()))
		{
			throw std::logic_error("No such element \'"+name+"\' found. At PugiXmlTree::removeElement.");
		}
	}


	void PugiXmlTree::removeAttribute(const std::string& name)
	{
		if (!mXmlNode.remove_attribute(name.c_str()))
		{
			throw std::logic_error("No such attribute \'"+name+"\' found. At PugiXmlTree::removeAttribute.");
		}
	}

} // namespace BFG

#endif