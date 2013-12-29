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

#ifndef GAMEOBJECT_FACTORY_H_
#define GAMEOBJECT_FACTORY_H_

#include <map>
#include <boost/weak_ptr.hpp>

#include <Event/Event.h>

#include <Model/Data/LevelConfig.h>
#include <Model/Data/ObjectParameters.h>
#include <Model/Data/FileHandleFactory.h>

#include <Model/Data/ConceptXml.h> 
#include <Model/Data/AdapterXml.h>
#include <Model/Data/ObjectXml.h>
#include <Model/Data/ValueXml.h>

#include <Model/Property/Plugin.h>

namespace BFG {

namespace Property {
	class ConceptFactory;
}

struct Adapter;
struct CameraParameter;
class Environment;
class GameObject;
struct Module;

class MODEL_API GameObjectFactory
{
public:
	typedef std::map<std::string, std::map<std::string, GameHandle> > GoModuleMapT;
	typedef std::map<std::string, boost::weak_ptr<GameObject> > GoMapT;

	GameObjectFactory(Event::Lane& lane,
	                  const LevelConfig& files,
	                  const Property::PluginMapT& propertyPlugins,
	                  boost::shared_ptr<Environment> environment,
	                  GameHandle stateHandle);

	boost::shared_ptr<GameObject>
	createGameObject(const ObjectParameter& parameters);

	boost::shared_ptr<GameObject>
	createCamera(const CameraParameter& cameraParameter,
                     const std::string& parentObject);

	//! Applies Connection logic of already created objects 
	void applyConnection(const ObjectParameter& parameters);

	const GoMapT& names() const { return mGameObjects; }

private:
	//! \brief Creates a new GameObject without Modules (thus "empty").
	boost::shared_ptr<GameObject>
	createEmptyGameObject(const ObjectParameter& parameter);

	boost::shared_ptr<Module>
	createModule(const std::string& moduleConcept, GameHandle moduleHandle);
	
	void
	attachModuleTo(boost::shared_ptr<GameObject> gameObject,
	               boost::shared_ptr<Module> module,
	               BFG::ModuleParametersT moduleParameter,
	               std::map<std::string, BFG::GameHandle>& moduleNameHandleMap);
	
	std::vector<Adapter>
	createAdapters(ModuleParametersT moduleParameter) const;
	
	void addConceptsTo(boost::shared_ptr<Module> module,
	                   const ConceptConfigT conceptParameter) const;
	
	Event::Lane& mLane;

	GoModuleMapT mGoModules;
	GoMapT mGameObjects;

	const Property::PluginMapT& mPropertyPlugins;

	boost::shared_ptr<Environment> mEnvironment;

	typedef FileHandleFactory<AdapterXml> AdapterXmlHandleT;
	typedef FileHandleFactory<ConceptXml> ConceptXmlHandleT;
	typedef FileHandleFactory<ObjectXml> ModuleXmlHandleT;
	typedef FileHandleFactory<ValueXml> ValueXmlHandleT;
	 
	AdapterXmlHandleT mAdapterParameters;
	ConceptXmlHandleT mConceptParameters;
	ModuleXmlHandleT mModuleParameters;
	ValueXmlHandleT mValueParameters;

	GameHandle mStateHandle;
};

} // namespace BFG

#endif
