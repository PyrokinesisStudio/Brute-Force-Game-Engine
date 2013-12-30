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

#include <Base/Logger.h>
#include <Core/GameHandle.h>

#include <Model/Adapter.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Module.h>
#include <Model/Property/Concepts/Camera.h> // struct CameraParameter

#include <Physics/Event_fwd.h>
#include <View/Enums.hh>
#include <View/CameraCreation.h>
#include <View/ObjectCreation.h>

namespace BFG {

GameObjectFactory::GameObjectFactory(Event::Lane& lane,
                                     const LevelConfig& files,
                                     const Property::PluginMapT& propertyPlugins,
                                     boost::shared_ptr<Environment> environment,
                                     GameHandle stateHandle) :
mLane(lane),
mPropertyPlugins(propertyPlugins),
mEnvironment(environment),
mAdapterParameters(files.mAdapters),
mConceptParameters(files.mConcepts),
mModuleParameters(files.mModules),
mValueParameters(files.mProperties),
mStateHandle(stateHandle)
{
	assert(environment && "GameObjectFactory: You must preserve a constructed Environment object");
}


GameHandle checkGoHandle(GameHandle handle)
{

	if (handle != NULL_HANDLE)
		return handle;
	
	return generateHandle();
}

void GameObjectFactory::createPhysical(ModulePtr module, 
                                       GameHandle goHandle, 
                                       std::vector<Physics::ModuleCreationParams>& physicsParameters)
{
	using Property::ValueId;
	auto pId = ValueId::ENGINE_PLUGIN_ID;
		
	// These values were calculated and set by attachModuleTo()
	auto position = module->mValues[ValueId(ID::PV_Position, pId)];
	auto orientation = module->mValues[ValueId(ID::PV_Orientation, pId)];
	auto density = module->mValues[ValueId(ID::PV_Density, pId)];

	BFG::Property::Value mesh = std::string();
	BFG::Property::Value cmStr = std::string();

	module->getValue(ValueId(ID::PV_PhysicalMesh, pId), mesh);
	if(!module->getValue(ValueId(ID::PV_CollisionMode, pId), cmStr))
	{
		cmStr = ID::asStr(ID::CM_Disabled);
	}

	physicsParameters.push_back
	(
		Physics::ModuleCreationParams
		(
			goHandle,
			module->getHandle(),
			mesh,
			density,
			ID::asCollisionMode(cmStr),
			position,
			orientation
		)
	);
}

void GameObjectFactory::createVisual(ModulePtr module, 
                                     GameHandle goHandle, 
                                     std::vector<View::ObjectCreation>& viewParameters)
{
	using Property::ValueId;
	auto pId = ValueId::ENGINE_PLUGIN_ID;
		
	// These values were calculated and set by attachModuleTo()
	auto position = module->mValues[ValueId(ID::PV_Position, pId)];
	auto orientation = module->mValues[ValueId(ID::PV_Orientation, pId)];
	
	BFG::Property::Value mesh = std::string();

	module->getValue(ValueId(ID::PV_VisualMesh, pId), mesh);

	viewParameters.push_back
	(
		View::ObjectCreation
		(
			goHandle==module->getHandle() ? NULL_HANDLE : goHandle,
			module->getHandle(),
			mesh,
			position,
			orientation
		)
	);
}

boost::shared_ptr<GameObject>
GameObjectFactory::createGameObject(const ObjectParameter& parameter)
{
	auto gameObject = createEmptyGameObject(parameter);
	auto goHandle = gameObject->getHandle();

	auto modules = mModuleParameters.requestConfig(parameter.mType);
	
	if (!modules)
		throw std::runtime_error("GameObjectFactory::createGameObject(): "
			"Type \"" + parameter.mType + "\" not found!");

	// In order to connect Modules together, we need the GameHandles of
	// previously created modules.
	std::map<std::string, GameHandle> moduleNameHandleMap;

	GameHandle moduleHandle = goHandle;

	std::vector<Physics::ModuleCreationParams> physicsParameters;
	std::vector<View::ObjectCreation> viewParameters;

	auto moduleIt = modules->mModules.begin();
	for (; moduleIt != modules->mModules.end(); ++moduleIt, moduleHandle = generateHandle())
	{
		auto mc = (*moduleIt);
		auto module = createModule(mc->mConcept, moduleHandle);
		setCustomValues(module, mc->mCustomValues);
		
		// Store GameHandle for later use
		moduleNameHandleMap[mc->mName] = module->getHandle();
		
		attachModuleTo(gameObject, module, mc, moduleNameHandleMap);

		auto begin = module->mPropertyConcepts.begin();
		auto end = module->mPropertyConcepts.end();

		if (std::find(begin, end, "Physical") != end)
			createPhysical(module, goHandle, physicsParameters);
		
		if (std::find(begin, end, "Visual") != end)
			createVisual(module, goHandle, viewParameters);
	}

	if (!physicsParameters.empty())
	{
		physicsParameters[0].mVelocity = parameter.mLinearVelocity;
		physicsParameters[0].mRotationVelocity = parameter.mAngularVelocity;
		mLane.emit(ID::PE_CREATE_OBJECT, physicsParameters);
	}

	if (!viewParameters.empty())
		mLane.emit(ID::VE_CREATE_OBJECT, viewParameters, mStateHandle);

	mGoModules[parameter.mName] = moduleNameHandleMap;
	mGameObjects[parameter.mName] = gameObject;

	parameter.mStorage.call(mLane.createSubLane());

	return gameObject;
}

void GameObjectFactory::setCustomValues(ModulePtr module, PropertyConfigT values)
{
	auto valueIt = values->mValueParameters.begin();

	for (; valueIt != values->mValueParameters.end(); ++valueIt)
	{
		PropertyParametersT valueParameter = *valueIt;
		ValueId vId = Property::symbolToValueId(valueParameter->mName, mPropertyPlugins);
		module->mValues[vId] = valueParameter->mValue;
	}
}

boost::shared_ptr<GameObject>
GameObjectFactory::createEmptyGameObject(const ObjectParameter& p)
{
	//! \hack No design made for init location values. So we just added it to the value store.
	auto pId = Property::ValueId::ENGINE_PLUGIN_ID;
	auto values = p.mGoValues;

	values.insert(std::make_pair(ValueId(ID::PV_Position, pId), p.mLocation.position));
	values.insert(std::make_pair(ValueId(ID::PV_Orientation, pId), p.mLocation.orientation));
	
	boost::shared_ptr<BFG::GameObject> go(
		new GameObject
		(
			mLane,
			checkGoHandle(p.mHandle),
			p.mName,
			values,
			mPropertyPlugins,
			mEnvironment
		)
	);

	mEnvironment->addGameObject(go);
	return go;
}

boost::shared_ptr<Module>
GameObjectFactory::createModule(const std::string& moduleConcept, GameHandle moduleHandle)
{
	if (moduleConcept.empty())
		throw std::runtime_error("No concept defined.");

	ConceptConfigT conceptParameter = mConceptParameters.requestConfig(moduleConcept);
	
	boost::shared_ptr<Module> module(new Module(moduleHandle));
	addConceptsTo(module, conceptParameter);

	return module;
}

void GameObjectFactory::attachModuleTo(boost::shared_ptr<BFG::GameObject> gameObject, boost::shared_ptr<Module> module, BFG::ModuleParametersT moduleParameter,  std::map<std::string, BFG::GameHandle>& moduleNameHandleMap)
{
	auto pId = ValueId::ENGINE_PLUGIN_ID;
	BFG::Property::Value mesh = std::string();
	
	bool isVirtual = !module->getValue(ValueId(ID::PV_PhysicalMesh, pId), mesh);
	
	if (isVirtual)
	{
		gameObject->attachModule(module);
		return;
	}

	GameHandle parentHandle;
	if (moduleParameter->mConnection.mConnectedExternToModule.empty())
		parentHandle = NULL_HANDLE;
	else
		parentHandle = moduleNameHandleMap[moduleParameter->mConnection.mConnectedExternToModule];
		
	std::vector<Adapter> adapters = createAdapters(moduleParameter);

	gameObject->attachModule
	(
		module,
		adapters,
		moduleParameter->mConnection.mConnectedLocalAt,
		parentHandle,
		moduleParameter->mConnection.mConnectedExternAt
	);
}

std::vector<Adapter>
GameObjectFactory::createAdapters(ModuleParametersT moduleParameter) const
{
	std::vector<Adapter> adapters;

	if (!moduleParameter->mAdapter.empty())
	{
		const AdapterConfigT adapterParameter = mAdapterParameters.requestConfig(moduleParameter->mAdapter);
		AdapterConfig::AdapterParameterListT::const_iterator adapterIt = adapterParameter->mAdapters.begin();

		for (; adapterIt != adapterParameter->mAdapters.end(); ++adapterIt)
		{
			AdapterParametersT adapterParameter = *adapterIt;

			Adapter adapter;
			adapter.mParentPosition = adapterParameter->mPosition;
			adapter.mParentOrientation = adapterParameter->mOrientation;
			adapter.mIdentifier = adapterParameter->mId;

			adapters.push_back(adapter);
		}
	}
	
	return adapters;
}

void GameObjectFactory::addConceptsTo(boost::shared_ptr<Module> module, const ConceptConfigT conceptParameter) const
{
	auto conceptIt = conceptParameter->mConceptParameters.begin();

	for (; conceptIt != conceptParameter->mConceptParameters.end(); ++conceptIt)
	{
		ConceptParametersT conceptParameter = *conceptIt;
		
		auto valueConfig = mValueParameters.requestConfig(conceptParameter->mProperties);
		auto valueIt = valueConfig->mValueParameters.begin();

		for (; valueIt != valueConfig->mValueParameters.end(); ++valueIt)
		{
			PropertyParametersT valueParameter = *valueIt;
			ValueId vId = Property::symbolToValueId(valueParameter->mName, mPropertyPlugins);
			module->mValues[vId] = valueParameter->mValue;
		}

		module->mPropertyConcepts.push_back(conceptParameter->mName);
	}
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

	std::vector<Physics::ModuleCreationParams> physicsParameters;

	physicsParameters.push_back
	(
		Physics::ModuleCreationParams
		(
			camHandle,
			camHandle,
			"Cube.mesh",
			50.0f,
			ID::CM_Disabled
		)
	);
	
	Physics::ObjectCreationParams ocp(camHandle, Location());
	mLane.emit(ID::PE_CREATE_OBJECT, physicsParameters);

	ObjectParameter op;
	op.mName = "Camera";
	op.mHandle = camHandle;

	auto camera = createEmptyGameObject(op);

	// Create Root Module
	boost::shared_ptr<Module> camModule(new Module(camHandle));
	camModule->mPropertyConcepts.push_back("Physical");
	camModule->mPropertyConcepts.push_back("Camera");

	camModule->mValues[ValueId(ID::PV_CameraMode, ValueId::ENGINE_PLUGIN_ID)] = cameraParameter.mMode;
	camModule->mValues[ValueId(ID::PV_CameraOffset, ValueId::ENGINE_PLUGIN_ID)] = cameraParameter.mOffset;

	camera->attachModule(camModule);

	View::CameraCreation cc(camHandle, NULL_HANDLE, cameraParameter.mFullscreen, 0, 0, parentHandle);
	mLane.emit(ID::VE_CREATE_CAMERA, cc, mStateHandle);
	mLane.emit(ID::GOE_SET_CAMERA_TARGET, parentHandle, camHandle);

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

} // namespace BFG
