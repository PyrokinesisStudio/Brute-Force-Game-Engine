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

#ifndef BFG_NETWORKMAIN_H
#define BFG_NETWORKMAIN_H

#ifdef _MSC_VER
#pragma warning (push)
// "class foo needs to have dll-interface to be used by clients of class bar"
#pragma warning (disable:4251)
#endif

#include <Base/LibraryMainBase.h>
#include <Event/Event.h>
#include <Network/Defs.h>

namespace BFG {
namespace Network {

class Server;
class Client;

class NETWORK_API Main : public Base::LibraryMainBase<Event::Lane>
{
public:
	Main(u8 mode);
	~Main();

private:
	void main(Event::Lane& lane);
	
	BFG::u8 mMode;
	
	boost::scoped_ptr<Server> mServer;
	boost::scoped_ptr<Client> mClient;
};

} // namespace Network
} // namespace BFG

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#endif
