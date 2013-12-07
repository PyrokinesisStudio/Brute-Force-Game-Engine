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

#ifndef LEVEL_OF_DETAIL_FACTORY_H_
#define LEVEL_OF_DETAIL_FACTORY_H_

#include <boost/shared_ptr.hpp>

#include <Base/Singleton.h>

#include <Model/Defs.h>
#include <Model/LevelOfDetailDefault.h>

namespace BFG {

//! The background of this class is that the LoDAlgroithm definition should be loaded only once!
class MODEL_API LevelOfDetailFactory : Base::CSingletonT<LevelOfDetailFactory>
{
	friend class Base::CLazyObject;

public:
	DECLARE_SINGLETON(LevelOfDetailFactory)

	//! Shared ptr because LevelOfDetail is an RAII object which loads automatically from LevelOfDetail.xml.
	//! Make a raw copy means to load and parse again.
	boost::shared_ptr<LevelOfDetailDefault> mLodDefault;

private:
	LevelOfDetailFactory();
};


} // namespace BFG

#endif