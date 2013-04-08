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

#ifndef NETWORKED_H_
#define NETWORKED_H_

#include <Core/ClockUtils.h>
#include <Model/Property/Concept.h>
#include <Network/Network_fwd.h>
#include <Physics/Event_fwd.h>

namespace BFG
{

class Networked : public Property::Concept
{
public:
	Networked(GameObject& owner, PluginId pid);
	~Networked();

	void onNetworkEvent(Network::DataPacketEvent* e);
	void onPhysicsEvent(Physics::Event* e);

private:
	void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);

	void internalOnEvent(EventIdT action,
	                     Property::Value payload,
	                     GameHandle module,
	                     GameHandle sender);

	void onPosition(const v3& newPosition);
	void onOrientation(const qv4& newOrientation);
	void onVelocity(const Physics::VelocityComposite& newVelocity);
	void onRotationVelocity(const Physics::VelocityComposite& newVelocity);

	void onSynchronizationMode(ID::SynchronizationMode mode);
	void onGhostMode(bool enable);

	bool receivesData() const;
	bool sendsData() const;

	void sendPosition() const;
	void sendOrientation() const;
	void sendVelocity(const v3& newVelocity) const;
	void sendRotationVelocity(const v3& newRotationVelocity) const;
	
	std::vector<ID::PhysicsAction> mPhysicsActions;
	std::vector<ID::NetworkAction> mNetworkActions;

	ID::SynchronizationMode mSynchronizationMode;

	Physics::FullSyncData mLastSentDelta;

	Physics::InterpolationDataV3  mLastPhysicsPosition;
	Physics::InterpolationDataQv4 mLastPhysicsOrientation;

	bool mInitialized;

	Clock::StopWatch mTimer;
	Clock::StopWatch mFullSyncTimer;

	bool mUpdatePosition;
	bool mUpdateOrientation;
	
	GameHandle mGhost;
	
	v3  mExtrapolatedPositionDelta;
	qv4 mExtrapolatedOrientationDelta;
};

} // namespace BFG

#endif