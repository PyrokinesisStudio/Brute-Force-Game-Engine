/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2013 Brute-Force Games GbR

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

#ifndef BFG_CORE_GAMEHANDLE_H
#define BFG_CORE_GAMEHANDLE_H

#include <string>

#include <Core/Defs.h>
#include <Core/Types.h>

namespace BFG {

typedef BFG::u32 GameHandle;

const static GameHandle NULL_HANDLE(0);

//! Generates a unique general purpose handle
//! \return The new handle
BFG_CORE_API GameHandle generateHandle();

//! Generates a unique general purpose handle with included ownership
//! \return The new handle
BFG_CORE_API GameHandle generateNetworkHandle();

//! Used to serialize handles to strings
BFG_CORE_API std::string stringify(GameHandle handle);

//! Used to deserialize handles from strings
BFG_CORE_API GameHandle destringify(const std::string& handle);

} // namespace BFG

#endif
