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

#ifndef MESHCONTROL
#define MESHCONTROL

#include <BaseFeature.h>

#include <OgreResourceGroupManager.h>

#include <Core/Path.h>

#include <OpenSaveDialog.h>
#include <SharedData.h>

namespace Tool
{

class MeshControl : public BaseFeature
{
public:
	MeshControl(boost::shared_ptr<SharedData> data) :
	BaseFeature("Mesh", true),
	mData(data)
	{
		BFG::Path p;

		mDialog.setDialogInfo
		(
			"Load Mesh",
			"Load",
			 MyGUI::newDelegate(this, &MeshControl::onLoadOk)
		);

		mDialog.setRestrictions
		(
			p.Get(BFG::ID::P_GRAPHICS_MESHES),
			true,
			".mesh"
		);
	}

	virtual ~MeshControl()
	{
	}

	virtual void load();
	virtual void unload();

	virtual void activate();
	virtual void deactivate();

protected:

private:
	void onLoadOk(MyGUI::Widget* w);

	boost::shared_ptr<SharedData> mData;
	OpenSaveDialog mDialog;
	std::string mPath;
}; // class MeshControl

} // namespace Tool
#endif