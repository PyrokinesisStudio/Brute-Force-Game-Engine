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

#ifndef TEXTURECONTROL
#define TEXTURECONTROL

#include <BaseFeature.h>

#include <SharedData.h>
#include <TextureUnit.h>


namespace Tool
{

class TextureControl : public BaseFeature
{
public:
	TextureControl(boost::shared_ptr<SharedData> data) :
	BaseFeature("Texture", true),
	mData(data)
	{
	}

	virtual ~TextureControl()
	{
		unload();
	}

	virtual void load();
	virtual void unload();

	virtual void activate();
	virtual void deactivate();

	virtual void update(const Ogre::FrameEvent& evt);

private:

	void onCloseClicked(MyGUI::Window*, const std::string& button);

	void checkForChanges();

	boost::shared_ptr<SharedData> mData;

	MyGUI::ScrollView* mTextureUnitBox;
	std::vector<boost::shared_ptr<TextureUnit> > mTextureUnits;

	BFG::GameHandle mActiveHandle;
	Ogre::SubEntity* mActiveSubEntity;
}; // class TextureControl

} // namespace Tool
#endif
