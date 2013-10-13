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

#ifndef BFG_CONTROLLER_VIP_COMPONENT_SEQUENCER_H
#define BFG_CONTROLLER_VIP_COMPONENT_SEQUENCER_H

namespace BFG {
namespace Controller_ { 
namespace Vip { 

template <typename Parent>
class Sequencer : public Parent
{
public:
	explicit Sequencer(typename Parent::EnvT& env) :
		Parent(env),
		mSequence(env.mSequence),
		mFilter(env.mFilter)
	{		
	}
	
	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		if (mFilter == bs ||
		    mFilter == ID::BS_Both)
		{
			mLastSequence.push_back(std::string::value_type(code));

			while (mLastSequence.length() > mSequence.length())
				mLastSequence.erase(mLastSequence.begin());
					  
			if (mSequence == mLastSequence)
				Parent::emit(Parent::getResult());
		}

		Parent::FeedButtonData(dt,bs,code);
	}
	
private:
	std::string mSequence;
	std::string mLastSequence;
	
	ID::ButtonState mFilter;
};

} // namespace Vip
} // namespace Controller_
} // namespace BFG

#endif
