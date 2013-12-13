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

#ifndef BASEFEATURE
#define BASEFEATURE

#include <MyGUI.h>

#include <OgreFrameListener.h>

#include <Event/Event.h>
#include <Controller/ControllerEvents.h>

namespace Tool
{

class BaseFeature
{
public:
	BaseFeature(const std::string& name,
	            const bool needButton = false) :
	mActive(false),
	mLoaded(false),
	mName(name),
	mMenuButton(NULL)
	{
		mGui = MyGUI::Gui::getInstancePtr();

		if (needButton)
			createButton();
	}

	virtual ~BaseFeature()
	{
		destroyButton();
	}

	virtual void load() = 0;
	virtual void unload() = 0;
	virtual bool isLoaded() {return mLoaded;}

	virtual void activate() = 0;
	virtual void deactivate() = 0;
	virtual bool isActive() {return mActive;}

	virtual void update(const Ogre::FrameEvent&){}

protected:

	void loadLayout(const std::string& filename, MyGUI::VectorWidgetPtr& container);

	MyGUI::Gui* mGui;
	MyGUI::VectorWidgetPtr mContainer;

	bool mActive;
	bool mLoaded;

private:

	void onButtonClicked(MyGUI::Widget*);

	void createButton();
	void destroyButton();

	std::string mName;
	MyGUI::Button* mMenuButton;

}; // class BaseFeature

} // namespace Tool

#endif 
