/**
\page PositionUpdate The Position Update Process

\msc
	hscale="1.6";
	ApplicationState,Sector,GameObject,Physical,ThrustControl,RenderObject,PhysicsObject,PhysicsManager;
	|||;
	ApplicationState <<= ApplicationState [label="onTick()", URL="\ref BFG::State::onTick()"];
	ApplicationState => Sector [label="update()", URL="\ref BFG::Sector::internalUpdate()"];
		Sector => GameObject [label="update()", URL="\ref BFG::GameObject::internalUpdate()"];
			GameObject => Physical [label="synchronize()", URL="\ref BFG::Physical::internalSynchronize()"];
				Physical <= Physical [label="synchronizeView()", URL="\ref BFG::Physical::synchronizeView()"];
				Physical => GameObject [label="getValue(PV_LOCATION)", URL="\ref BFG::GameObject::getValue()"];
				GameObject >> Physical [label="Location", URL="\ref BFG::Location"];
				Physical -> RenderObject [label="VE_UPDATE_POSITION"];
				RenderObject <= RenderObject [label="updatePosition()", URL="\ref BFG::View::RenderObject::updatePosition()"];
				RenderObject abox RenderObject [label="Update OGRE Position", textbgcolour="#C1DAD6"];
			GameObject => ThrustControl [label="update()", URL="\ref BFG::ThrustControl::internalSynchronize()"];
				ThrustControl -> PhysicsObject [label="PE_APPLY_FORCE"];
				PhysicsObject <= PhysicsObject [label="onForce()", URL="\ref BFG::Physics::PhysicsObject::onForce()"];
				PhysicsObject abox PhysicsObject [label="Update ODE Position", textbgcolour="#C1DAD6"];
	ApplicationState->PhysicsManager [label="PE_STEP"];
		PhysicsManager => PhysicsObject [label="prepareOdeStep()", URL="\ref BFG::Physics::PhysicsObject::prepareOdeStep()"];
		PhysicsManager <= PhysicsManager [label="dWorldQuickStep()"];
				PhysicsManager => PhysicsObject [label="sendFullSync()", URL="\ref BFG::Physics::PhysicsObject::sendFullSync()"];
				PhysicsObject -> Physical [label="PE_FULL_SYNC"];
				Physical => GameObject [label="setValue(PV_LOCATION)", URL="\ref BFG::GameObject::setValue()"];
\endmsc

A few things which are not displayed within this diagram:

  - update() is not called directly, instead \ref BFG::Managed::update() "update()" calls pure virtual \ref BFG::Managed::internalUpdate() "internalUpdate()" instead.
  - getValue() is not called directly, instead \ref BFG::Property::Concept::getGoValue() "Concept::getGoValue()" is called, which in turn calls \ref BFG::GameObject::getValue() "GameObject::getValue()".

*/
