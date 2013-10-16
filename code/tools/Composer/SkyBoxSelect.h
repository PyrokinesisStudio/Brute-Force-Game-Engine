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

#ifndef SKYBOXSELECT
#define SKYBOXSELECT

#include <BaseFeature.h>
#include <Event/Event.h>
#include <SharedData.h>

namespace Tool
{

class SkyBoxSelect : public BaseFeature
{
public:
	SkyBoxSelect(BFG::Event::SubLanePtr sublane, boost::shared_ptr<SharedData> data) :
	BaseFeature("SkyBox", true),
	mSubLane(sublane),
	mData(data)
	{
	}

	virtual void load();
	virtual void unload();

	virtual void activate();
	virtual void deactivate();

private:

	void onSkySelected(MyGUI::ListBox* list, size_t index);

	std::vector<Ogre::TextureUnitState*> findTextureAlias(const std::string alias);

	Ogre::TextureUnitState* findAliasInMaterial(const std::string alias,
	                                            const std::string matName);

	BFG::Event::SubLanePtr mSubLane;
	boost::shared_ptr<SharedData> mData;

	MyGUI::ListBox* mList;

}; // class SkyBoxSelect

} // namespace Tool
#endif