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

#ifndef OBJECT_PARAMETERS_H_
#define OBJECT_PARAMETERS_H_

#include <string>

#include <boost/shared_ptr.hpp>

#include <Core/CharArray.h>
#include <Core/GameHandle.h>
#include <Core/Location.h>
#include <Core/Types.h>
#include <Core/Math.h>

#include <Model/Data/Connection.h>


namespace BFG {

//! This struct saves the interpreted object data which is not(!) defined by PropertyConcepts.
struct ObjectParameter
{
	typedef std::map
	<
		std::string,
		ObjectParameter
	> MapT;

	ObjectParameter() :
		mHandle(NULL_HANDLE),
		mLinearVelocity(v3::ZERO),
		mAngularVelocity(v3::ZERO) {}

	ObjectParameter(XmlTreeT tree) :
		mHandle(NULL_HANDLE),
		mLinearVelocity(v3::ZERO),
		mAngularVelocity(v3::ZERO)
	{
		load(tree);
	}

	ObjectParameter(const CharArray512T& in)
	{
		u32 offset = arrayToValue(mHandle, in, 0);
		offset = arrayToValue(mName, in, offset);
		offset = arrayToValue(mType, in, offset);
		offset = arrayToValue(mLocation.position, in, offset);
		offset = arrayToValue(mLocation.orientation, in, offset);
		offset = arrayToValue(mLinearVelocity, in, offset);
		offset = arrayToValue(mAngularVelocity, in, offset);
		offset = arrayToValue(mConnection.mConnectedLocalAt, in, offset);
		offset = arrayToValue(mConnection.mConnectedExternToGameObject, in, offset);
		offset = arrayToValue(mConnection.mConnectedExternToModule, in, offset);
		offset = arrayToValue(mConnection.mConnectedExternAt, in, offset);
	}

	void serialize(CharArray512T& output) const
	{
		u32 offset = valueToArray(mHandle, output, 0);
		
		offset = valueToArray(mName.size(), output, offset);
		offset = valueToArray(mName, output, offset);
		
		offset = valueToArray(mType.size(), output, offset);
		offset = valueToArray(mType, output, offset);

		offset = valueToArray(mLocation.position, output, offset);
		offset = valueToArray(mLocation.orientation, output, offset);

		offset = valueToArray(mLinearVelocity, output, offset);
		offset = valueToArray(mAngularVelocity, output, offset);

		offset = valueToArray(mConnection.mConnectedLocalAt, output, offset);
		offset = valueToArray(mConnection.mConnectedExternToGameObject.size(), output, offset);
		offset = valueToArray(mConnection.mConnectedExternToGameObject, output, offset);
		offset = valueToArray(mConnection.mConnectedExternToModule.size(), output, offset);
		offset = valueToArray(mConnection.mConnectedExternToModule, output, offset);
		offset = valueToArray(mConnection.mConnectedExternAt, output, offset);
	}


	GameHandle mHandle;
	std::string mName;
	std::string mType;
	Location mLocation;
	v3 mLinearVelocity;
	v3 mAngularVelocity;
	Connection mConnection;

	protected:
	
	void load(XmlTreeT tree)
	{
		mType = tree->attribute("type");
		mName = tree->attribute("name");

		try
		{
			mLocation.position = loadVector3(tree->child("Position"));
			parseConnection(tree->child("Connection")->elementData(), mConnection);
		}
		catch (std::exception& e)
		{
			throw std::logic_error(e.what()+std::string(" At ObjectParameter::load(...)"));
		}

		try
		{
			mLocation.orientation = loadQuaternion(tree->child("Orientation"));
		}
		catch (std::exception& e1)
		{
			try
			{
				v3 ori = loadVector3(tree->child("Orientation"));
				mLocation.orientation = eulerToQuaternion(ori);
			}
			catch (std::exception& e2)
			{
				throw std::logic_error(e2.what()+std::string(" At ObjectParameter::load(...). Catched before: ")+e1.what());
			}
		}
	}
};

typedef std::vector<boost::shared_ptr<ObjectParameter> > ObjectDefinitionsT;

} // namespace BFG

#endif
