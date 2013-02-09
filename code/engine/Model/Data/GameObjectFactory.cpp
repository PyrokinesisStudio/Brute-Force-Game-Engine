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

#include <Model/Data/GameObjectFactory.h>

#include <boost/foreach.hpp>

#include <Core/Utils.h> // generateHandle()

#include <Model/Adapter.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Module.h>
#include <Model/Property/Concepts/Camera.h> // struct CameraParameter

#include <Physics/Event.h>
#include <View/Event.h>

namespace BFG {
namespace Loader {

GameObjectFactory::GameObjectFactory(EventLoop* loop,
                                     const LevelConfig& files,
                                     const Property::PluginMapT& propertyPlugins,
                                     boost::shared_ptr<Environment> environment,
                                     GameHandle stateHandle) :
Emitter(loop),
mPropertyPlugins(propertyPlugins),
mEnvironment(environment),
mStateHandle(stateHandle),
mModuleParameters(files.mModules),
mAdapterParameters(files.mAdapters),
mConceptParameters(files.mConcepts),
mValueParameters(files.mProperties)
{
	assert(environment && "GameObjectFactory: You must preserve a constructed Environment object");
}


GameHandle checkGoHandle(GameHandle handle)
{

	if (handle != NULL_HANDLE)
		return handle;
	
	return generateHandle();
}


boost::shared_ptr<GameObject>
GameObjectFactory::createGameObject(const ObjectParameter& parameter)
{
	GameHandle goHandle = checkGoHandle(parameter.mHandle);

	// Create the PhysicsObject
	Physics::ObjectCreationParams ocp(goHandle, parameter.mLocation);
	emit<Physics::Event>(ID::PE_CREATE_OBJECT, ocp);

	// First Module is always root
	bool isRoot = true;

	boost::shared_ptr<GameObject> gameObject;

	// In order to connect Modules together, we need the GameHandles of
	// previously created modules.
	std::map<std::string, GameHandle> moduleNameHandleMap;

	ModuleConfigT modules = mModuleParameters.requestConfig(parameter.mType);

	if (!modules)
		throw std::runtime_error("GameObjectFactory::createGameObject(): "
			"Type \"" + parameter.mType + "\" not found!");

	ModuleConfig::ModulesT::iterator moduleIt = modules->mModules.begin();
	
	for (; moduleIt != modules->mModules.end(); ++moduleIt)
	{
		ModuleParametersT moduleParameter = *moduleIt;

		GameHandle moduleHandle;

		// The root module and its owner GameObject must share the same GameHandle.
		if (isRoot)
			moduleHandle = goHandle;
		else
			moduleHandle = generateHandle();

		boost::shared_ptr<Module> module(new Module(moduleHandle));

		bool isVirtual = moduleParameter->mMesh.empty();

		if (! isVirtual)		
		{
			// Physical representation			
			Physics::ModuleCreationParams mcp
			(
				goHandle,
				moduleHandle,
				moduleParameter->mMesh,
				moduleParameter->mCollision,
				v3::ZERO,
				qv4::IDENTITY,
				moduleParameter->mDensity
			);

			emit<Physics::Event>(ID::PE_ATTACH_MODULE, mcp);

			// Visual representation
			View::ObjectCreation oc
			(
				NULL_HANDLE,
				moduleHandle,
				moduleParameter->mMesh,
				v3::ZERO,
				qv4::IDENTITY
			);

			emit<View::Event>(ID::VE_CREATE_OBJECT, oc, mStateHandle);

			if (!isRoot)
			{
				emit<View::Event>(ID::VE_ATTACH_OBJECT, moduleHandle, goHandle);
			}
		}

		ConceptConfigT conceptParameter = mConceptParameters.requestConfig(moduleParameter->mConcept);
		
		// Load "Concepts" and their "Values"
		if (!conceptParameter)
		{
			if (moduleParameter->mConcept.empty())
			{
				throw std::runtime_error
					("GameObjectFactory::createGameObject(): Missing concept specification for object type \"" + parameter.mType + "\".");
			}
		}

		ConceptConfig::ConceptParameterListT::iterator conceptIt = conceptParameter->mConceptParameters.begin();

		for (; conceptIt != conceptParameter->mConceptParameters.end(); ++conceptIt)
		{
			ConceptParametersT conceptParameter = *conceptIt;
			
			PropertyConfigT valueConfig = mValueParameters.requestConfig(conceptParameter->mProperties);
			PropertyConfig::PropertyParametersListT::iterator valueIt = valueConfig->mValueParameters.begin();

			for (; valueIt != valueConfig->mValueParameters.end(); ++valueIt)
			{
				PropertyParametersT valueParameter = *valueIt;
				ValueId vId = Property::symbolToValueId(valueParameter->mName, mPropertyPlugins);
				module->mValues[vId] = valueParameter->mValue;
			}

			module->mPropertyConcepts.push_back(conceptParameter->mName);
		}

		// Store GameHandle for later use
		moduleNameHandleMap[moduleParameter->mName] = moduleHandle;

		if (isRoot)
		{
			if (! isVirtual)
			{
				emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, parameter.mLinearVelocity, goHandle);
				emit<Physics::Event>(ID::PE_UPDATE_ROTATION_VELOCITY, parameter.mAngularVelocity, goHandle);
			}

			// Create GameObject
			gameObject.reset
			(
				new GameObject
				(
					loop(),
					goHandle,
					parameter.mName,
					mPropertyPlugins,
					mEnvironment
				)
			);

			// Register it in the environment
			mEnvironment->addGameObject(gameObject);
		}

		GameHandle parentHandle;

		if (moduleParameter->mConnection.mConnectedExternToModule.empty())
			parentHandle = NULL_HANDLE;
		else
			parentHandle = moduleNameHandleMap[moduleParameter->mConnection.mConnectedExternToModule];

		if (isVirtual)
		{
			gameObject->attachModule(module);
		}
		else
		{
			std::vector<Adapter> adapterVector;

			if (!moduleParameter->mAdapter.empty())
			{
				AdapterConfigT adapterParameter = mAdapterParameters.requestConfig(moduleParameter->mAdapter);
				AdapterConfig::AdapterParameterListT::iterator adapterIt = adapterParameter->mAdapters.begin();

				for (; adapterIt != adapterParameter->mAdapters.end(); ++adapterIt)
				{
					AdapterParametersT adapterParameter = *adapterIt;

					Adapter adapter;
					adapter.mParentPosition = adapterParameter->mPosition;
					adapter.mParentOrientation = adapterParameter->mOrientation;
					adapter.mIdentifier = adapterParameter->mId;

					adapterVector.push_back(adapter);
				}
			}

			gameObject->attachModule
			(
				module,
				adapterVector,
				moduleParameter->mConnection.mConnectedLocalAt,
				parentHandle,
				moduleParameter->mConnection.mConnectedExternAt
			);
		}

		isRoot = false;	
	}

	mGoModules[parameter.mName] = moduleNameHandleMap;
	mGameObjects[parameter.mName] = gameObject;

	return gameObject;
}

boost::shared_ptr<GameObject>
GameObjectFactory::createCamera(const CameraParameter& cameraParameter,
                                const std::string& parentObject)
{
	GoMapT::const_iterator it =
		mGameObjects.find(parentObject);

	if (it == mGameObjects.end())
	{
		throw std::runtime_error
		(
			"GameObjectFactory: Unable to find \"" + parentObject + "\" for "
			"use as camera position. Skipping the creation of this camera."
		);
	}

	GameHandle parentHandle = it->second.lock()->getHandle();

	GameHandle camHandle = generateHandle();

	Physics::ObjectCreationParams ocp(camHandle, Location());
	emit<Physics::Event>(ID::PE_CREATE_OBJECT, ocp);

	Physics::ModuleCreationParams mcp
	(
		camHandle,
		camHandle,
		"Cube.mesh",
		ID::CM_Disabled,
		v3::ZERO,
		qv4::IDENTITY,
		50.0f
	);

	emit<Physics::Event>(ID::PE_ATTACH_MODULE, mcp);

	boost::shared_ptr<GameObject> camera
	(
		new GameObject
		(
			loop(),
			camHandle,
			"Camera",
			mPropertyPlugins,
			mEnvironment
		)
	);

	boost::shared_ptr<Module> camModule(new Module(camHandle));
	camModule->mPropertyConcepts.push_back("Physical");
	camModule->mPropertyConcepts.push_back("Camera");

	camModule->mValues[ValueId(ID::PV_CameraMode, ValueId::ENGINE_PLUGIN_ID)] = cameraParameter.mMode;
	camModule->mValues[ValueId(ID::PV_CameraOffset, ValueId::ENGINE_PLUGIN_ID)] = cameraParameter.mOffset;

	camera->attachModule(camModule);

	View::CameraCreation cc(camHandle, NULL_HANDLE, cameraParameter.mFullscreen, 0, 0);
	emit<View::Event>(ID::VE_CREATE_CAMERA, cc, mStateHandle);
	emit<GameObjectEvent>(ID::GOE_SET_CAMERA_TARGET, parentHandle, camHandle);

	mEnvironment->addGameObject(camera);

	return camera;
}

void GameObjectFactory::applyConnection(const ObjectParameter& parameters)
{
	if (! parameters.mConnection.good())
		return;

	GoMapT::const_iterator it =
		mGameObjects.find(parameters.mConnection.mConnectedExternToGameObject);

	// Check if parent gameobject exists at all
	if (it == mGameObjects.end())
	{
		errlog << "GameObjectFactory: Unable to connect \"" << parameters.mName
		       << "\" to " << "\""
		       << parameters.mConnection.mConnectedExternToGameObject
		       << "\" since the latter wasn't found.";
		return;
	}

	boost::shared_ptr<GameObject> parent = it->second.lock();

	GameHandle parentModule = 
		mGoModules[parameters.mConnection.mConnectedExternToGameObject]
		          [parameters.mConnection.mConnectedExternToModule];

	u32 parentAdapter = parameters.mConnection.mConnectedExternAt;

	boost::shared_ptr<GameObject> child = mGameObjects[parameters.mName].lock();

	u32 childAdapter = parameters.mConnection.mConnectedLocalAt;

	parent->attachModule
	(
		child,
		child->rootAdapters(),
		childAdapter,
		parentModule,
		parentAdapter
	);
}

} // namespace Loader
} // namespace BFG