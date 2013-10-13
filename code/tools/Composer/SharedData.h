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

#ifndef SHAREDDATA
#define SHAREDDATA

#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

#include <OgreSubEntity.h>

#include <Core/Types.h>
#include <Model/Adapter.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <View/RenderObject.h>

#include <Connection.h>
#include <Module.h>

namespace Tool {

struct SharedData
{
	struct TextureUnitChange
	{
		std::string mTextureName;
		bool mEnabled;
	};

	BFG::GameHandle mState;
	BFG::GameHandle mCamera;
	BFG::GameHandle mActiveMesh;
	BFG::GameHandle mRootMesh;

	typedef boost::shared_ptr<BFG::View::RenderObject> RenderObjT;
	typedef	std::map<BFG::GameHandle, RenderObjT> RenderObjectMapT;
	typedef std::map<BFG::GameHandle, std::string> MeshMapT;
	typedef std::map<BFG::GameHandle, std::string> HandleAdapterMapT;
	typedef std::map<std::string, std::vector<BFG::Adapter> > AdapterMapT;
	typedef std::vector<Tool::Module*> ModulesT;
	typedef std::vector<Tool::Connection*> ConnectionsT;

	RenderObjectMapT mRenderObjects;
	MeshMapT mMeshNames;

	Ogre::SubEntity* mSelectedSubEntity;
	std::string mMaterialName;

	AdapterMapT mAdapters;
	HandleAdapterMapT mObjectAdapters;

	ModulesT mModules;
	ConnectionsT mConnections;

	BFG::GameObject* mGameObject;

	boost::shared_ptr<BFG::Environment> mEnvironment;

	typedef boost::tuple<std::string, std::string> MaterialKeyT;
	typedef boost::shared_ptr<SharedData::TextureUnitChange> TucPtrT;
	typedef std::map<MaterialKeyT, TucPtrT> ChangeMapT;

	ChangeMapT mTextureChanges;

	SharedData() :
	mState(BFG::NULL_HANDLE),
	mCamera(BFG::NULL_HANDLE),
	mActiveMesh(BFG::NULL_HANDLE),
	mRootMesh(BFG::NULL_HANDLE),
	mSelectedSubEntity(NULL),
	mGameObject(NULL)
	{
		mEnvironment.reset(new BFG::Environment);
	}
};

} // namespace Tool
#endif