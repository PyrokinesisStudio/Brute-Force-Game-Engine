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

#ifndef BFG_PROPERTY_PARAMETERS_H_
#define BFG_PROPERTY_PARAMETERS_H_

#include <Core/XmlTree.h>

#include <Core/v3.h>
#include <Core/qv4.h>
#include <Core/strToBool.h>

#include <Model/Enums.hh>
#include <Model/Property/Value.h>


namespace BFG
{

struct PropertyParameters
{
	PropertyParameters(XmlTreeT tree)
	{
		load(tree);
	}
	
	std::string mName;
	Property::Value mValue;

protected:
	
	void load(XmlTreeT tree)
	{
		
		mName = tree->attribute("name");
		std::string type = tree->attribute("type");
		
		if (type == "v3")
		{
			mValue = loadVector3(tree);
		}
		else 
		if (type == "qv4")
		{
			mValue = loadQuaternion(tree);
		}
		else
		{
			mValue = stringToPropertyValue(tree->elementData());
		}
	}

	Property::Value stringToPropertyValue(const std::string& input)
	{
		Property::Value result;
	
		// is qv4
		try {
			qv4 output;
			stringToQuaternion4(input, output);
			result = output;
			return result;
		}
		catch (std::runtime_error) {}
	
		// is v3
		try {
			v3 output;
			stringToVector3(input, output);
			result = output;
			return result;
		}
		catch (std::runtime_error) {}

		// is integer
		try
		{
			result = boost::lexical_cast<s32>(input);
			return result;
		}
		catch (boost::bad_lexical_cast &) {}

		// is float
		try
		{
			result = boost::lexical_cast<f32>(input);
			return result;
		}
		catch (boost::bad_lexical_cast &) {}

		// is ID::CameraMode
		try {
			result = ID::asCameraMode(input);
			return result;
		}
		catch (std::out_of_range) {}
	
		// is bool
		bool b; // calling strToBool with result as param yields bad_anycast
		if (strToBool(input, b))
		{
			result = b;
			return result;
		}
	
		return result;
	}
};

typedef boost::shared_ptr<PropertyParameters> PropertyParametersT;

}

#endif
