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

#include <Model/GameObject.h>

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include <Base/Logger.h>
#include <Base/Cpp.h>
#include <Core/Math.h>

#include <Model/Environment.h>
#include <Model/Adapter.h>
#include <Model/Module.h>
#include <Model/Property/Concept.h>
#include <Model/Property/ConceptFactory.h>

#include <Physics/Event_fwd.h>

#include <View/Enums.hh>

using namespace boost;
using namespace boost::adaptors;

namespace BFG {

void throwIfEnginePropertiesNotFoundOrWrongId(const Property::PluginMapT& pm)
{
	typedef Property::PluginMapT::nth_index<1>::type PluginByName;
	PluginByName::const_iterator it = pm.get<1>().find("Engine Properties");
	if (it == pm.get<1>().end())
	{
		throw std::logic_error
			("Currently, GameObject still needs the original model properties");
	}
	else if ((*it)->id() != ValueId::ENGINE_PLUGIN_ID)
	{
		throw std::logic_error
			("PluginId of \"Engine Properties\" must be set to ValueId::ENGINE_PLUGIN_ID.");
	}
}

GameObject::GameObject(
	Event::Lane& lane,
	const GameHandle handle,
	const std::string& publicName,
	const Property::PluginMapT& propertyPlugins,
	boost::shared_ptr<Environment> environment) :
Managed(handle, publicName, ID::OT_GameObject),
mSubLane(lane.createSubLane()),
mEnvironment(environment),
mPropertyPlugins(propertyPlugins),
mDocked(false),
mActivated(false)
{
	throwIfEnginePropertiesNotFoundOrWrongId(propertyPlugins);

	GameObject::ChildT dummy = boost::shared_ptr<Module>();
	mDummy = boost::add_vertex(dummy, mModules);
	
	// Hack: Settings this in case Property::Concept::Networked is not used.
	//       Other Property Concepts may depend on this flag and crash if it
	//       isn't set yet.
	//
	//       Also, it should be possible to do such initializations when
	//       creating a GameObject (and not by an "event-setter" later).
	setValue(ID::PV_Remote, ValueId::ENGINE_PLUGIN_ID, false);

	mSubLane->connectV(ID::GOE_DETACH_MODULE, this, &GameObject::detachModule, handle);
}

GameObject::~GameObject()
{
	mEnvironment->removeGameObject(getHandle());

	mSubLane.reset();

	// Explicitly clear() since unregisterEvent() calls may still happen
	mEventDemands.clear();
}

void GameObject::attachModule(GameObject::ChildT managed,
                              const std::vector<Adapter>& adapters,
                              const u32 childAdapterID,
                              const GameHandle parent,
                              const u32 parentAdapterID)
{
	// Add Module
	VD childVd = boost::add_vertex(managed, mModules);
	
	// Add Adapters (pointing to dummies)
	for (size_t i=0; i < adapters.size(); ++i)
	{
		boost::add_edge(childVd, mDummy, adapters[i], mModules);
	}

	//!	\note
	//! Now, check if this new module is connected to something.
	//!	If so, we need to merge the edge data of the two adapters from each
	//!	module to a new one (which will become the edge between)
	//! If not, it'll become the root module.

	if (parent != NULL_HANDLE) // is module
	{
		VD parentVd = findVertex(parent);
		
		if (parentVd == mDummy)
			throw std::logic_error
				("Parent-module handle for new child not found!");
		
		connectChildToParent(parentVd, parentAdapterID, childVd, childAdapterID);
	}
	else // is root
	{
		// When root gets created, the number of vertices must be exactly 2.
		// If not, someone probably tries to attach a second root module.
		// These 2 vertices are Dummy and Root.
		if (num_vertices(mModules) != 2)
		{
			std::stringstream ss;
			ss << "GameObject::attachModule: No orphaned non-root modules are "
			      "allowed. Every child module must have a parent.";
			throw std::runtime_error(ss.str());
		}

		assert(managed->getHandle() == getHandle() &&
			"The root module must have the same handle as its container GO");

		mRootAdapters = adapters;
	}

	// Here, we're calculating the absolute position for the new
	// Module/GameObject towards the root Module of this ship by adding vectors
	// from previous parents.
	v3  fromRootToNewOne;
	qv4 finalOrientation;
	vectorToModuleFromRoot(childVd, fromRootToNewOne, finalOrientation);

	if (managed->getObjectType() == ID::OT_Module)
	{
		boost::shared_ptr<Module> mod = boost::shared_static_cast<Module>(managed);

		Property::ValueId positionId(ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);
		Property::ValueId orientationId(ID::PV_Orientation, ValueId::ENGINE_PLUGIN_ID);

		mod->mValues[positionId] = fromRootToNewOne;
		mod->mValues[orientationId] = finalOrientation;
		
		mSubLane->emit(ID::PE_UPDATE_MODULE_POSITION, fromRootToNewOne,	managed->getHandle());
		mSubLane->emit(ID::PE_UPDATE_MODULE_ORIENTATION, finalOrientation, managed->getHandle());

		// Add to hasModuleWithHandle()-Cache
		mModuleHandles.push_back(managed->getHandle());
		
		notifyPropertyConcepts(boost::shared_static_cast<Module>(managed));
	}
	else if (managed->getObjectType() == ID::OT_GameObject)
	{
		boost::shared_ptr<GameObject> go = boost::shared_static_cast<GameObject>(managed);
		
		connectOtherGameObject(go, fromRootToNewOne, finalOrientation);
	}
}

void GameObject::attachModule(boost::shared_ptr<Module> module)
{
	// Add to hasModuleWithHandle()-Cache
	mModuleHandles.push_back(module->getHandle());

	mVirtualModules[module->getHandle()] = module;

	notifyPropertyConcepts(module);
}

void GameObject::detachModule(GameHandle handle)
{
	VD vd = findVertex(handle);
	if (vd == mDummy)
		return;
		
	// Restore Adapter for parent in the tree
	boost::graph_traits<ModuleTreeT>::in_edge_iterator iei, iei_end; 
	boost::tie(iei, iei_end) = boost::in_edges(vd, mModules);

	for (; iei != iei_end; ++iei)
	{
		Adapter parentAdapter = mModules[*iei];
		parentAdapter.mChildOrientation = qv4::IDENTITY;
		parentAdapter.mChildPosition = v3::ZERO;

		boost::add_edge(boost::source(*iei, mModules), mDummy, parentAdapter, mModules);
	}

	// Perform type specific operations
	ID::ObjectType ot = mModules[vd]->getObjectType();

	if (ot == ID::OT_Module)
	{	
		// Remove it from the hasModuleWithHandle()-Cache
		std::vector<GameHandle>::iterator it;
		it = std::find(mModuleHandles.begin(), mModuleHandles.end(), handle);
		mModuleHandles.erase(it);
	}
	else if (ot == ID::OT_GameObject)
	{
		// We must put it to same location it has had before as a Module.
		v3 fromRootToGo;
		qv4 finalOrientation;
		vectorToModuleFromRoot(vd, fromRootToGo, finalOrientation);

		const v3& ownPosition = getValue<v3>(ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);
		const qv4& ownOrientation = getValue<qv4>(ID::PV_Orientation, ValueId::ENGINE_PLUGIN_ID);

		// Apply World Transform
		v3 newPos(ownPosition + ownOrientation * fromRootToGo);
		qv4 newOri(ownOrientation * finalOrientation);
		
		boost::shared_ptr<GameObject> go =
			boost::shared_static_cast<GameObject>(mModules[vd]);

		disconnectOtherGameObject(go, newPos, newOri);
	}

	deleteVertex(vd);
}

bool GameObject::satisfiesRequirement(Property::ConceptId concept) const
{
	return mConcepts.find(concept) != mConcepts.end();
}

void GameObject::debugOutput(std::string& output) const
{
	std::stringstream ss;

	ss << "Debug output for GameObject\n"
	   << "ID: " << getHandle() << ", Name: \"" << getPublicName() << "\".\n";
	ss << "Status: " << (mActivated?"Activated":"Deactivated") << "\n";
	ss << "\n";
	ss << "[Property Plugins]\n";

	for_each(mPropertyPlugins.get<1>(), ss << *boost::lambda::_1 << "\n");

	ss << "\n";
	ss << "[Property Concepts]\n";

	ConceptContainerT::const_iterator it1 = mConcepts.begin();

	for (; it1 != mConcepts.end(); ++it1)
		ss << "- " << it1->first << " @ "
		   << it1->second << "\n";

	ss << "\n";
	ss << "[Requested Events]\n";

	EventDemandContainerT::const_iterator it2 = mEventDemands.begin();
	
	for (; it2 != mEventDemands.end(); ++it2)
		ss << "- " << it2->first << " by "
		   << it2->second->concept() << "\n";
		
	ss << "\n";
	ss << "[Property Values]\n";
	
	Module::ValueStorageT::const_iterator it3 = mValues.begin();
	
	for (; it3 != mValues.end(); ++it3)
	{
		std::string valueId = valueIdToSymbol(it3->first, mPropertyPlugins);
		ss << "- " << valueId << " = "
		   << "[OUTPUT DEACTIVATED]"
//		   << it3->second
		   << "\n";
	}
	
	output = ss.str();
}

const boost::shared_ptr<Environment> GameObject::environment() const
{
	return mEnvironment;
}

const std::vector<Adapter>& GameObject::rootAdapters() const
{
	return mRootAdapters;
}

void GameObject::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	if (!mActivated)
		return;
	

	//! \see  GameObject::rebuildConceptUpdateOrder()
	UpdateOrderContainerT::const_iterator it = mConceptUpdateOrder.begin();
	for(; it != mConceptUpdateOrder.end(); ++it)
	{
		boost::shared_ptr<Property::Concept> pc = it->lock();
		pc->update(timeSinceLastFrame);

	}
}

void GameObject::activate()
{
	mActivated = true;

	sendValueUpdate(ID::PV_Position, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_Orientation, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_Velocity, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_RelativeVelocity, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_RotationVelocity, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_RelativeRotationVelocity, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_Mass, ValueId::ENGINE_PLUGIN_ID);
	sendValueUpdate(ID::PV_Inertia, ValueId::ENGINE_PLUGIN_ID);
}

void GameObject::sendValueUpdate(Property::ValueId::VarIdT varId,
                                 Property::PluginId pluginId)
{
	ValueId valueId(varId, pluginId);
	
	if (mActivated)
		mSubLane->emit(ID::GOE_VALUE_UPDATED, valueId, getHandle());
}

void GameObject::internalSynchronize()
{
	if (!mActivated)
		return;

	//! \see  GameObject::rebuildConceptUpdateOrder()
	UpdateOrderContainerT::const_iterator it = mConceptUpdateOrder.begin();
	for(; it != mConceptUpdateOrder.end(); ++it)
	{
		boost::shared_ptr<Property::Concept> pc = it->lock();
		pc->synchronize();
	}
}

void GameObject::setValue(Property::ValueId::VarIdT varId,
                          Property::PluginId pluginId,
                          const Property::Value& value)
{
	ValueId valueId(varId, pluginId);
	mValues[valueId] = value;

	sendValueUpdate(varId, pluginId);
}

//! \note This MUST be called from within a Concept constructor!
//!       Since this will be evaluated by calling rebuildConceptUpdateOrder()
//!       right after the creation of all Concepts.
void GameObject::setRequirement(Property::ConceptId self,
                                Property::ConceptId other)
{
	mConceptDependencies.insert(std::make_pair(self, other));
}

void GameObject::deleteVertex(GameObject::VD vd)
{
	if (vd == mDummy)
		return;
	boost::graph_traits<ModuleTreeT>::out_edge_iterator ei, ei_end;
	boost::tie(ei, ei_end) = boost::out_edges(vd, mModules);
	
	for (; ei != ei_end; ++ei)
	{
		deleteVertex(boost::target(*ei, mModules));
	}

	boost::clear_vertex(vd, mModules);
	boost::remove_vertex(vd, mModules);
}

GameObject::VD GameObject::findVertex(GameHandle handle) const
{
	boost::graph_traits<ModuleTreeT>::vertex_iterator vi, vi_end; 
	boost::tie(vi, vi_end) = boost::vertices(mModules);

	for (; vi != vi_end; ++vi)
	{
		if (*vi == mDummy)
			continue;

		GameHandle current = boost::get<boost::shared_ptr<Managed> >
			(mModules[*vi])->getHandle();

		if (current == handle)
			return *vi;
	}
	return mDummy;
}

void GameObject::vectorToModuleFromRoot(VD module, v3& vecResult, qv4& oriResult) const
{
	using namespace boost;
	
	// Follow the path to root traversing in-edges and
	// add directional vectors to vecResult.
	VD Root    = findVertex(getHandle());
	VD current = module;
	
	std::vector<Adapter> adapters;

	while (current != Root)
	{
		graph_traits<GameObject::ModuleTreeT>::in_edge_iterator iei, iei_end;
		tuples::tie(iei, iei_end) = in_edges(current, mModules);

		assert(iei != iei_end && "GameObject::vectorToModuleFromRoot: "
			"Got a vertex which has no in-edges but is not root either!");

		adapters.push_back(mModules[*iei]);

		current = source(*iei, mModules);
	}

	BFG::vectorToModuleFromRoot(adapters, vecResult, oriResult);
}

void GameObject::notifyPropertyConcepts(boost::shared_ptr<Module> module)
{
	Module::ConceptStorageT::const_iterator it =
		module->mPropertyConcepts.begin();

	for (; it != module->mPropertyConcepts.end(); ++it)
	{
		// Add, if not already created
		if (!satisfiesRequirement(*it))
		{
			mConcepts[*it] = Property::createConcept(*it, *this, mPropertyPlugins);
		}

		// Notify the PCs the new module needs
		mConcepts[*it]->onModuleAttached(module);
	}
	
	rebuildConceptUpdateOrder();
}

void GameObject::connectChildToParent(VD parentVd,
                                      u32 parentAdapterID,
                                      VD childVd,
                                      u32 childAdapterID)
{
	// Copy both old edges and delete them afterwards from the tree
	Adapter parentEdge;
	Adapter childEdge;

	boost::remove_out_edge_if
	(
		parentVd,
		boost::bind
		(
			&GameObject::hasSameIndex,
			boost::ref(*this),
			_1,
			parentAdapterID,
			boost::ref(parentEdge)
		),
		mModules
	);

	boost::remove_out_edge_if
	(
		childVd,
		boost::bind
		(
			&GameObject::hasSameIndex,
			boost::ref(*this),
			_1,
			childAdapterID,
			boost::ref(childEdge)
		),
		mModules
	);
	
	// Check if adapters were found at all
	if (! parentEdge.good())
	{
		std::stringstream ss;
		ss << "Couldn't find parent Adapter:" << parentAdapterID
		   << " for connecting it to child Adapter:" << childAdapterID
		   << ". Happened in GO:" << getHandle() << ".\n"
		   << "\n"
		   << *this;
		throw std::logic_error("GameObject::connectChildToParent: " + ss.str());
	}
	else if (! childEdge.good())
	{
		std::stringstream ss;
		ss << "Couldn't find child Adapter:" << childAdapterID
		   << " for connecting it to parent Adapter:" << parentAdapterID
		   << ". Happened in GO:" << getHandle() << ".";
		throw std::logic_error("GameObject::connectChildToParent: " + ss.str());
	}

	// Create new Edge
	parentEdge.mChildOrientation = childEdge.mParentOrientation;
	parentEdge.mChildPosition = childEdge.mParentPosition;

	boost::add_edge(parentVd, childVd, parentEdge, mModules);

	boost::print_graph(mModules);
}

void GameObject::connectOtherGameObject(boost::shared_ptr<GameObject> other,
                                        const v3& position,
                                        const qv4& orientation)
{
	if (other->mDocked)
	{
		std::stringstream ss;
		ss << "GameObject::connectOtherGameObject:"
		      " Tried to attach a GameObject to this one which already is"
		      " connected to some other GameObject!";
		throw std::runtime_error(ss.str());
	}

	other->mDocked = true;

	const GameHandle rootHandle = getHandle();
	const GameHandle child = other->getHandle();

	// Connect Physical Components
	Physics::ObjectAttachmentParams oap(rootHandle, child, position, orientation);
	mSubLane->emit(ID::PE_ATTACH_OBJECT, oap);

	// Connect View Components
	mSubLane->emit(ID::VE_ATTACH_OBJECT, child, rootHandle);
	mSubLane->emit(ID::VE_UPDATE_POSITION, position, child);
	mSubLane->emit(ID::VE_UPDATE_ORIENTATION, orientation, child);
}

void GameObject::disconnectOtherGameObject(boost::shared_ptr<GameObject> other,
                                           const v3& position,
                                           const qv4& orientation)
{
	other->mDocked = false;

	const GameHandle rootHandle = getHandle();
	const GameHandle child = other->getHandle();

	// Disconnect Physical Components
	Physics::ObjectAttachmentParams oap(rootHandle, child, position, orientation);
	mSubLane->emit(ID::PE_DETACH_OBJECT, oap);

 	// Disconnect View Components
	mSubLane->emit(ID::VE_DETACH_OBJECT, Event::Void(), child);
	mSubLane->emit(ID::VE_UPDATE_POSITION, position, child);
	mSubLane->emit(ID::VE_UPDATE_ORIENTATION, orientation, child);
}

bool GameObject::hasModuleWithHandle(GameHandle handle) const
{
	if (handle == getHandle())
		return true;

	std::vector<GameHandle>::const_iterator it;
	it = std::find(mModuleHandles.begin(), mModuleHandles.end(), handle);
	return it != mModuleHandles.end();
}

bool GameObject::hasSameIndex(ModuleTreeT::edge_descriptor e,
                              u32 Index,
                              Adapter& oldAdapter) const
{
	const Adapter& current = mModules[e];
	bool ret = current.mIdentifier == Index;

	if (ret)
		oldAdapter = current;

	return ret;
}

namespace detail {

//! Utility for GameObject::rebuildPcUpdateOrder()
template <typename OrderContainerT>
class DfsOrderVisitor : public boost::dfs_visitor<>
{
public:
	DfsOrderVisitor(OrderContainerT& order) :
			mOrder(order)
	{
	}
	template <typename Vertex, typename Graph>
	void finish_vertex(Vertex u, const Graph & g) const
	{
		mOrder.push_back(g[u]);
	}

	OrderContainerT& mOrder;
};

} // namespace detail

void GameObject::rebuildConceptUpdateOrder()
{
	//! Property::Concept update tree hierarchy
	//! http://stackoverflow.com/questions/2244580/find-boost-bgl-vertex-by-a-key
	typedef boost::labeled_graph
	<
		boost::adjacency_list
		<
			boost::vecS,
			boost::vecS,
			boost::directedS,
			UpdateOrderContainerT::value_type
		>,
	    std::string
	> DependenciesTreeT;

	typedef DependenciesTreeT::graph_type GraphT;
	typedef GraphT::vertex_descriptor VertexDescT;

	DependenciesTreeT conceptDependenciesTree;
	GraphT& g = conceptDependenciesTree.graph();
	
	// Building a nice dependency tree (std::multimap -> boost::graph)
	ConceptDependenciesT::const_iterator it = mConceptDependencies.begin();
	for (; it != mConceptDependencies.end(); ++it)
	{
		ConceptContainerT::const_iterator itSelf = mConcepts.find(it->first);
		ConceptContainerT::const_iterator itOther = mConcepts.find(it->second);

		// \todo Check validity of itOther
		// \todo Add check for missing concepts

		VertexDescT selfVd = boost::add_vertex(it->first, conceptDependenciesTree);
		VertexDescT otherVd = boost::add_vertex(it->second, conceptDependenciesTree);

		// Attaching the particular Concept pointer to the tree vertices, which
		// safes some time later when updating concepts (in the correct order).
		g[selfVd] = itSelf->second;
		g[otherVd] = itOther->second;

		boost::add_edge(selfVd, otherVd, conceptDependenciesTree);
	}

	//! http://www.boost.org/doc/libs/1_46_1/boost/graph/topological_sort.hpp
	//! http://www.boost.org/doc/libs/1_46_1/libs/graph/doc/bgl_named_params.html
	//! http://www.boost.org/doc/libs/1_46_1/libs/graph/doc/depth_first_search.html

	mConceptUpdateOrder.clear();
	typedef detail::DfsOrderVisitor<UpdateOrderContainerT> MyVisitor;
	boost::depth_first_search(g, boost::visitor(MyVisitor(mConceptUpdateOrder)));

	// Copy every Concept of mConcepts to mConceptUpdateOrder if it's not already there
	
	ConceptContainerT::const_iterator conceptIt = mConcepts.begin();
	for (; conceptIt != mConcepts.end(); ++conceptIt)
	{
		UpdateOrderContainerT::const_iterator result = std::find_if
		(
			mConceptUpdateOrder.begin(),
			mConceptUpdateOrder.end(),
			boost::bind(&UpdateOrderContainerT::value_type::lock, _1) == conceptIt->second
		);
		if (result == mConceptUpdateOrder.end())
			mConceptUpdateOrder.push_back(conceptIt->second);
	}
}

Event::SubLanePtr GameObject::subLane()
{
	return mSubLane;
}

void vectorToModuleFromRoot(const std::vector<Adapter>& adapters,
                            v3& vecResult,
                            qv4& oriResult)
{
	vecResult  = v3::ZERO;
	oriResult  = qv4::IDENTITY;

	std::vector<Adapter>::const_reverse_iterator rIt = adapters.rbegin();
	for (; rIt != adapters.rend(); ++rIt)
	{
		const Adapter& a = *rIt;
		v3 parentAdapterPos = oriResult * a.mParentPosition;
		qv4 parentAdapterOri = oriResult * a.mParentOrientation;

		dbglog << "xAngle: " << angleBetween(a.mChildOrientation.xAxis(), parentAdapterOri.xAxis()) * RAD2DEG;
		dbglog << "yAngle: " << angleBetween(a.mChildOrientation.yAxis(), parentAdapterOri.yAxis()) * RAD2DEG;
		dbglog << "zAngle: " << angleBetween(a.mChildOrientation.zAxis(), parentAdapterOri.zAxis()) * RAD2DEG;

		qv4 firstOri = rotationTo(a.mChildOrientation.yAxis(), -parentAdapterOri.yAxis());

 		qv4 childOri = firstOri * a.mChildOrientation;
 		
		// determine whether to turn left or right (using parent x-axis as the 
		// normal of a plane and checking if the child z-axis is in front or 
		// behind the plane)
		f32 angleSign = sign(boost::geometry::dot_product(parentAdapterOri.xAxis(), childOri.zAxis()));
		
		qv4 secondOri;
		f32 angle = angleBetween(childOri.zAxis(), parentAdapterOri.zAxis());
		fromAngleAxis(secondOri, angleSign * angle, childOri.yAxis());
	
		oriResult = secondOri*firstOri;

		// just for debug output purposes		
		childOri = oriResult * a.mChildOrientation;
		dbglog << "xAngle: " << angleBetween(childOri.xAxis(), parentAdapterOri.xAxis()) * RAD2DEG;
		dbglog << "yAngle: " << angleBetween(childOri.yAxis(), parentAdapterOri.yAxis()) * RAD2DEG;
		dbglog << "zAngle: " << angleBetween(childOri.zAxis(), parentAdapterOri.zAxis()) * RAD2DEG;

		v3 childPos = oriResult * a.mChildPosition;
		dbglog << "ChildPos: " << childPos;
		dbglog << "ParentPos: " << a.mParentPosition;
				
		vecResult += parentAdapterPos - childPos;
		dbglog << "VecResult: " << vecResult;
	}
}

std::ostream& operator << (std::ostream& lhs, const GameObject& rhs)
{
	std::string s;
	rhs.debugOutput(s);
	lhs << s;
	return lhs;
}

} // namespace BFG
