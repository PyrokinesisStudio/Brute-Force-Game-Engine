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

#include "MiniMain.h"

#include <OgreDefaultHardwareBufferManager.h>
#include <OgreRoot.h>

#include <Core/Path.h>

#include <View/Enums.hh>
#include <View/LoadMesh.h>

namespace BFG {
namespace View { 

MiniMain::MiniMain() :
mLane(NULL),
mOgreRoot(NULL)
{}

MiniMain::~MiniMain()
{
	mOgreRoot->shutdown();
	delete mOgreRoot;
}

void MiniMain::main(BFG::Event::Lane* lane)
{
	mLane = lane;
	
	mLane->connect(ID::VE_REQUEST_MESH, this, &MiniMain::onRequestMesh);
	
	initOgreForMeshLoading();
}

void MiniMain::initOgreForMeshLoading()
{
	Path p;
	const std::string meshPath = p.Get(ID::P_GRAPHICS_MESHES);

	mOgreRoot = new Ogre::Root("", "", p.Get(ID::P_LOGS) + "OgreMeshLoad.log");
	new Ogre::DefaultHardwareBufferManager();
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(meshPath, "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();	
}

void MiniMain::onRequestMesh(const std::string& meshName, GameHandle sender)
{
	const Mesh mesh = loadMesh(meshName);
	NamedMesh namedMesh(meshName, mesh);
	dbglog << "Processing mesh request \"" << meshName << "\" for #" << sender;
	mLane->emit(ID::VE_DELIVER_MESH, namedMesh, sender);
}

} // namespace View
} // namespace BFG

