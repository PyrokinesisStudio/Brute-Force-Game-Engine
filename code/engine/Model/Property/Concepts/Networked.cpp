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

#include <Model/Property/Concepts/Networked.h>

#include <boost/foreach.hpp>

#include <Base/Logger.h>

#include <Core/CharArray.h>
#include <Core/GameHandle.h>
#include <Core/Math.h>

#include <Model/Data/ObjectParameters.h>

#include <Network/Network.h>

#include <Physics/Physics.h>

#include <View/Enums.hh>

namespace BFG {

Networked::Networked(GameObject& owner, PluginId pid) :
Property::Concept(owner, "Networked", pid),
mSynchronizationMode(ID::SYNC_MODE_NETWORK_NONE),
mInitialized(false),
mTimer(Clock::milliSecond),
mFullSyncTimer(Clock::milliSecond),
mUpdatePosition(false),
mUpdateOrientation(false),
mGhost(NULL_HANDLE),
mExtrapolatedPositionDelta(v3::ZERO),
mExtrapolatedOrientationDelta(qv4::IDENTITY)
{
	require("Physical");
	
	subLane()->connect(ID::PE_POSITION, this, &Networked::onPosition, ownerHandle());
	subLane()->connect(ID::PE_ORIENTATION, this, &Networked::onOrientation, ownerHandle());
	subLane()->connect(ID::PE_VELOCITY, this, &Networked::onVelocity, ownerHandle());
	subLane()->connect(ID::PE_ROTATION_VELOCITY, this, &Networked::onRotationVelocity, ownerHandle());

	subLane()->connect(ID::NE_RECEIVED, this, &Networked::onReceived, ownerHandle());
	
	subLane()->connect(ID::GOE_SYNCHRONIZATION_MODE, this, &Networked::onSynchronizationMode, ownerHandle());
	subLane()->connect(ID::GOE_GHOST_MODE, this, &Networked::onGhostMode, ownerHandle());

	mTimer.start();
	mFullSyncTimer.start();
}

Networked::~Networked()
{}

void Networked::internalUpdate(quantity<si::time, f32> /*timeSinceLastFrame*/)
{
	// Updates per second (cl_updaterate)
	const u32 UPDATES_PER_SECOND = 20;

	// Server: Wait time in ms till next update
	const u32 UPDATE_DELAY = 1000 / UPDATES_PER_SECOND;

	// Server: The positional distance an object must have to be updated (in meters)
	const f32 MAX_POSITION_DELTA = 0.1f;
	
	// Server/Client: The orientational distance an object must have to be updated (in rad)
	const f32 MAX_ORIENTATION_DELTA = 0.08727f;
	
	// Client: The threshold with regard to velocity for the maximum allowed
	//         distance between the extrapolated position and the received one.
	const f32 MAX_EXTRAPOLATED_POSITION_DELTA = 0.1f;
	
	const u32 FULLSYNCUPDATE_DELAY = 5000;

	if (!receivesData()) // server
	{
		if (mFullSyncTimer.stop() > FULLSYNCUPDATE_DELAY)
		{
			sendPosition();
			mLastSentDelta.get<0>() = mLastPhysicsPosition.get<2>();
			sendOrientation();
			mLastSentDelta.get<1>() = mLastPhysicsOrientation.get<2>();
			sendVelocity(getGoValue<v3>(ID::PV_Velocity, pluginId()));
			sendRotationVelocity(getGoValue<v3>(ID::PV_RotationVelocity, pluginId()));

			mFullSyncTimer.restart();
		}

		if (mTimer.stop() < UPDATE_DELAY)
			return;

		mTimer.restart();

		if (mUpdatePosition)
		{
			if (nearEnough(mLastPhysicsPosition.get<2>(), mLastSentDelta.get<0>(), MAX_POSITION_DELTA))
				return;

			sendPosition();
			mLastSentDelta.get<0>() = mLastPhysicsPosition.get<2>();
			mUpdatePosition = false;
		}

		if (mUpdateOrientation)
		{
			if (angleBetween(mLastPhysicsOrientation.get<2>(), mLastSentDelta.get<1>()) < MAX_ORIENTATION_DELTA)
				return;

			sendOrientation();
			mLastSentDelta.get<1>() = mLastPhysicsOrientation.get<2>();
			mUpdateOrientation = false;
		}


	}
	else // client
	{
		const v3& ownPosition = getGoValue<v3>(ID::PV_Position, pluginId());
		const qv4& ownOrientation = getGoValue<qv4>(ID::PV_Orientation, pluginId());
		
		
		//Location go = getGoValue<Location>(ID::PV_Location, pluginId());

		if (!mInitialized)
		{
			mLastPhysicsPosition.get<2>() = ownPosition;
			mLastPhysicsOrientation.get<2>() = ownOrientation;
			mInitialized = true;
		}

		if (mUpdatePosition)
		{
			v3 velocity = getGoValue<v3>(ID::PV_Velocity, pluginId());
			f32 speed = length(velocity);

			if (!nearEnough(ownPosition + mExtrapolatedPositionDelta, mLastPhysicsPosition.get<2>(), speed * MAX_EXTRAPOLATED_POSITION_DELTA))
			{
				dbglog << "Updating since distance was " << length(ownPosition + mExtrapolatedPositionDelta - mLastPhysicsPosition.get<2>());
				dbglog << "Speed was " << speed;
				subLane()->emit(ID::PE_INTERPOLATE_POSITION, mLastPhysicsPosition, ownerHandle());
			}
			mUpdatePosition = false;
		}
		if (mUpdateOrientation)
		{
			if(angleBetween(mLastPhysicsOrientation.get<2>(), ownOrientation * mExtrapolatedOrientationDelta) > MAX_ORIENTATION_DELTA)
			{
				dbglog << "AngleBetween: " << angleBetween(mLastPhysicsOrientation.get<2>(), ownOrientation * mExtrapolatedOrientationDelta);
				subLane()->emit(ID::PE_INTERPOLATE_ORIENTATION, mLastPhysicsOrientation, ownerHandle());
			}
			mUpdateOrientation = false;
		}
	}
}

void Networked::onPosition(const v3& newPosition)
{
	if (!sendsData())
		return;

	mLastPhysicsPosition = boost::make_tuple(0, 0, newPosition);
	//dbglog << "Networked:onPosition(original): " << newPosition;
	mUpdatePosition = true;
}

void Networked::onOrientation(const qv4& newOrientation)
{
	// Don't send if not either WRITE or RW
	if (!sendsData())
		return;

	mLastPhysicsOrientation = boost::make_tuple(0, 0, newOrientation);
	//dbglog << "Networked:onOrientation: " << newOrientation;
	mUpdateOrientation = true;
}

void Networked::onVelocity(const Physics::VelocityComposite& newVelocity)
{
	if (!sendsData())
		return;

	dbglog << "Networked:onVelocity: " << newVelocity.get<0>();

	sendVelocity(newVelocity.get<0>());

}

void Networked::onRotationVelocity(const Physics::VelocityComposite& newVelocity)
{
	if (!sendsData())
		return;

	dbglog << "Networked:onRotationVelocity: " << newVelocity.get<0>();

	sendRotationVelocity(newVelocity.get<0>());
}

void Networked::onReceived(const BFG::Network::DataPayload& payload)
{
	if (!receivesData())
		return;

	if (!mInitialized)
		return;

	dbglog << "Current Server Time: " << payload.mTimestamp;

	switch(payload.mAppEventId)
	{
	case ID::PE_UPDATE_POSITION:
	{
		assert(ownerHandle() == payload.mAppDestination);

		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		v3 v;
		stringToVector3(msg, v);
		dbglog << "Networked:onNetworkEvent: receivedPosition: " << v;

		// Update Ghost
		if (mGhost != NULL_HANDLE)
			subLane()->emit(ID::VE_UPDATE_POSITION, v, mGhost);
		
		// Only update if the new position is too different from our own calculated one.
		v3 velocity = getGoValue<v3>(ID::PV_Velocity, pluginId());
		f32 deltaTime = payload.mAge / 1000.0f;
		mExtrapolatedPositionDelta = velocity * deltaTime;
		
		mLastPhysicsPosition = boost::make_tuple(payload.mTimestamp, payload.mAge, v);
		mUpdatePosition = true;
		break;
	}
	case ID::PE_UPDATE_ORIENTATION:
	{
		assert(ownerHandle() == payload.mAppDestination);

		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		qv4 o;
		stringToQuaternion4(msg, o);
		dbglog << "Networked:onNetworkEvent: receivedOrientation: " << o;

		// Update Ghost
		if (mGhost != NULL_HANDLE)
			subLane()->emit(ID::VE_UPDATE_ORIENTATION, o, mGhost);

		v3 rotVelocity = getGoValue<v3>(ID::PV_RotationVelocity, pluginId());
		f32 deltaTime = payload.mAge / 1000.0f;
		// TODO: deltaTime and mExtrapolatedOrientationDelta are unused
		
		mLastPhysicsOrientation = boost::make_tuple(payload.mTimestamp, payload.mAge, o);
		mUpdateOrientation = true;
		break;
	}
	case ID::PE_UPDATE_VELOCITY:
	{
		assert(ownerHandle() == payload.mAppDestination);

		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		v3 v;
		stringToVector3(msg, v);
		dbglog << "Networked:onNetworkEvent: Velocity: " << v;
		subLane()->emit(ID::PE_UPDATE_VELOCITY, v, ownerHandle());
		break;
	}
	case ID::PE_UPDATE_ROTATION_VELOCITY:
	{
		assert(ownerHandle() == payload.mAppDestination);

		std::string msg(payload.mAppData.data(), payload.mAppDataLen);
		v3 v;
		stringToVector3(msg, v);
		dbglog << "Networked:onNetworkEvent: RotationVelocity: " << v;
		subLane()->emit(ID::PE_UPDATE_ROTATION_VELOCITY, v, ownerHandle());
		break;
	}
	default:
	{
		warnlog << "Networked::onNetworkEvent: Received unknown EventId: "
		        << payload.mAppEventId;
	}
	} // switch(payload.mAppEventId)
}


void Networked::onSynchronizationMode(ID::SynchronizationMode mode)
{
	mSynchronizationMode = mode;

	// Set Remote flag for whole GameObject
	setGoValue(ID::PV_Remote, pluginId(), receivesData());
	
	// Disable Collision for Remote Objects on all modules
	if (receivesData())
	{
		ModuleMapT::iterator it = mModules.begin();
		for (; it != mModules.end(); ++it)
			subLane()->emit(ID::PE_UPDATE_COLLISION_MODE, ID::CM_Disabled, it->first);
	}
	
	dbglog << "Networked: setting synchronization mode to " << mode;
}

void Networked::onGhostMode(bool enable)
{
	if (enable && mGhost == NULL_HANDLE)
	{
		ObjectParameter op;
		std::stringstream ss;
		ss << "Ghost of " << ownerHandle();
		op.mName = ss.str();
		op.mType = "Cube_Ghost";
		op.mLocation = v3(10000,10000,10000);  // far far away
		op.mLinearVelocity = v3::ZERO;

		mGhost = op.mHandle = generateHandle();
		subLane()->emit(ID::S_CREATE_GO, op);
		infolog << "Ghost mode enabled for " << ownerHandle();
	}
	else if (!enable)
	{
		subLane()->emit(ID::S_DESTROY_GO, mGhost);
		mGhost = NULL_HANDLE;
		infolog << "Ghost mode disabled for " << ownerHandle();
	}
}

bool Networked::receivesData() const
{
	// A client *reads* data.
	return mSynchronizationMode == ID::SYNC_MODE_NETWORK_READ ||
	       mSynchronizationMode == ID::SYNC_MODE_NETWORK_RW;
}

bool Networked::sendsData() const
{
	// A server *writes* data.
	return mSynchronizationMode == ID::SYNC_MODE_NETWORK_WRITE ||
	       mSynchronizationMode == ID::SYNC_MODE_NETWORK_RW;
}

void Networked::sendPosition() const
{
	std::stringstream ss;
	ss << mLastPhysicsPosition.get<2>();

	CharArray512T ca512 = stringToArray<512>(ss.str());

	BFG::Network::DataPayload payload
	(
		ID::PE_UPDATE_POSITION, 
		ownerHandle(),
		ownerHandle(),
		ss.str().length(),
		ca512
	);

	subLane()->emit(BFG::ID::NE_SEND_UDP, payload);
}

void Networked::sendOrientation() const
{
	std::stringstream ss;
	ss << mLastPhysicsOrientation.get<2>();

	CharArray512T ca512 = stringToArray<512>(ss.str());

	BFG::Network::DataPayload payload
	(
		ID::PE_UPDATE_ORIENTATION, 
		ownerHandle(),
		ownerHandle(),
		ss.str().length(),
		ca512
	);

	subLane()->emit(BFG::ID::NE_SEND_UDP, payload);
}

void Networked::sendVelocity(const v3& newVelocity) const
{
	std::stringstream ss;
	ss << newVelocity;

	CharArray512T ca512 = stringToArray<512>(ss.str());

	BFG::Network::DataPayload payload
	(
		ID::PE_UPDATE_VELOCITY, 
		ownerHandle(),
		ownerHandle(),
		ss.str().length(),
		ca512
	);

	subLane()->emit(BFG::ID::NE_SEND_UDP, payload);
}

void Networked::sendRotationVelocity(const v3& newRotationVelocity) const
{
	std::stringstream ss;
	ss << newRotationVelocity;

	CharArray512T ca512 = stringToArray<512>(ss.str());

	BFG::Network::DataPayload payload
	(
		ID::PE_UPDATE_ROTATION_VELOCITY, 
		ownerHandle(),
		ownerHandle(),
		ss.str().length(),
		ca512
	);

	subLane()->emit(BFG::ID::NE_SEND_UDP, payload);
}

} // namespace BFG
