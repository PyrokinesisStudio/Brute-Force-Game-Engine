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

#include <MeshControl.h>

#include <OgreRoot.h>

#include <Core/GameHandle.h>
#include <View/RenderObject.h>

namespace Tool
{

void MeshControl::load()
{
	if (mLoaded)
		return;

	mLoaded = true;
	deactivate();
}

void MeshControl::unload()
{
	if (!mLoaded)
		return;

	if (mActive)
		deactivate();
	
	mLoaded = false;
}

void MeshControl::activate()
{
	mDialog.setVisible(true);
	mActive = true;
}

void MeshControl::deactivate()
{
	mDialog.setVisible(false);
	mActive = false;
}

void MeshControl::onLoadOk(MyGUI::Widget* w)
{
	std::string folder = mDialog.getCurrentFolder();
	std::string meshName = mDialog.getFileName().substr(folder.size() + 1);

	if (!(mData->mRootMesh))
	{
		mData->mRootMesh = BFG::generateHandle();
	}

	mData->mRenderObjects[mData->mRootMesh].reset();
	mData->mRenderObjects[mData->mRootMesh].reset(new BFG::View::RenderObject
	(
		BFG::NULL_HANDLE,
		mData->mRootMesh,
		meshName,
		BFG::v3::ZERO,
		BFG::qv4::IDENTITY
	));

	mData->mMeshNames[mData->mRootMesh] = meshName;
	mData->mActiveMesh = mData->mRootMesh;

	deactivate();
}

} // namespace Tool
