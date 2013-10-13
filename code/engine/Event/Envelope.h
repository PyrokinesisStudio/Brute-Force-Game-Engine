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

#ifndef BFG_EVENT_ENVELOPE_H
#define BFG_EVENT_ENVELOPE_H

namespace BFG {
namespace Event { 

//! Makes all typenames within an Envelope type available in a scope.
//! \attention Do *not* use EnvelopeT as parameter. You may use _EnvelopeT or
//!            a type of a dependent scope like Mytype::EnvelopeT.
#define USING_ENVELOPE(type) \
	typedef          type                      EnvelopeT;       \
	typedef typename EnvelopeT::IdT            IdT;             \
	typedef typename EnvelopeT::DestinationIdT DestinationIdT;  \
	typedef typename EnvelopeT::SenderIdT      SenderIdT;

//! Struct containing event addressing type traits.
template <typename _IdT, typename _DestinationIdT, typename _SenderIdT>
struct BasicEnvelope
{
	typedef _IdT            IdT;
	typedef _DestinationIdT DestinationIdT;
	typedef _SenderIdT      SenderIdT;
};

} // namespace Event
} // namespace BFG

#endif
