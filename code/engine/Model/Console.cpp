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

#include <Model/Console.h>

#include <boost/lexical_cast.hpp>
#include <Physics/Enums.hh>
#include <Model/Enums.hh>

namespace BFG
{
	void initConsoleCommands(CommandPtrT commands)
	{
		registerCommand("debug po", &debugPhysicsObject, *commands);
		registerCommand("debug go", &debugPhysicsObject, *commands);
	}

	void debugPhysicsObject(const std::string& body, Event::SubLanePtr sublane)
	{
		try
		{
			auto handle = boost::lexical_cast<GameHandle>(body);
			sublane->emit(ID::PE_DEBUG, Event::Void(), handle);
		}
		catch (boost::bad_lexical_cast)
		{
			infolog << "Bad cast in console function debugPhysicsObject. " << body << " is probably not a number.";
		}
	}

	void debugGameObject(const std::string& body, Event::SubLanePtr sublane)
	{
		try
		{
			auto handle = boost::lexical_cast<GameHandle>(body);
			sublane->emit(ID::GOE_DEBUG, Event::Void(), handle);
		}
		catch (boost::bad_lexical_cast)
		{
			infolog << "Bad cast in console function debugGameObject. " << body << " is probably not a number.";
		}
	}

}
