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

#ifndef __SCORE_H_
#define __SCORE_H_

#include <Event/Event.h>
#include <View/HudElement.h>
#include <Core/Types.h>

namespace BFG {
namespace View {

class Score : public HudElement
{
public:
	Score(Event::SubLanePtr subLane);
	~Score();

private:
	virtual void internalUpdate(f32 time);

	void onUpperBarWin();
	void onLowerBarWin();
	void onScoreUpdate();

	Event::SubLanePtr mSubLane;
	s32 mUpperScore;
	s32 mLowerScore;
};

} // namespace View
} // namespace BFG

#endif //__PONG_STATE_H_
