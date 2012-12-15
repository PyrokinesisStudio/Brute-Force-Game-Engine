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

#include <Core/CharArray.h>
#include <Core/Math.h>
#include <Network/Event.h>
#include <Physics/Event.h>

namespace BFG {

Networked::Networked(GameObject& owner, PluginId pid) :
Property::Concept(owner, "Networked", pid),
mSynchronizationMode(ID::SYNC_MODE_NETWORK_NONE),
mInitialized(false),
mTimer(Clock::milliSecond),
mUpdatePosition(false),
mUpdateOrientation(false)
{
	require("Physical");

	mPhysicsActions.push_back(ID::PE_POSITION);
	mPhysicsActions.push_back(ID::PE_ORIENTATION);
	mPhysicsActions.push_back(ID::PE_VELOCITY);
	mPhysicsActions.push_back(ID::PE_ROTATION_VELOCITY);
	BOOST_FOREACH(ID::PhysicsAction action, mPhysicsActions)
	{
		loop()->connect(action, this, &Networked::onPhysicsEvent, ownerHandle());
	}

	mNetworkActions.push_back(ID::NE_RECEIVED);
	BOOST_FOREACH(ID::NetworkAction action, mNetworkActions)
	{
		loop()->connect(action, this, &Networked::onNetworkEvent, ownerHandle());
	}

	requestEvent(ID::GOE_SYNCHRONIZATION_MODE);

	mTimer.start();
}

Networked::~Networked()
{
	BOOST_FOREACH(ID::PhysicsAction action, mPhysicsActions)
	{
		loop()->disconnect(action, this);
	}
	BOOST_FOREACH(ID::NetworkAction action, mNetworkActions)
	{
		loop()->disconnect(action, this);
	}
}

void Networked::onNetworkEvent(Network::DataPacketEvent* e)
{
	if (!receivesData())
		return;

	switch(e->getId())
	{
	case ID::NE_RECEIVED:
	{
		const BFG::Network::DataPayload& payload = e->getData();

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
			
			mInterpolationData = boost::make_tuple(payload.mTimestamp, payload.mAge, v);
			mUpdatePosition = true;
			break;
		}
		case ID::PE_UPDATE_ORIENTATION:
		{
			assert(ownerHandle() == payload.mAppDestination);

			std::string msg(payload.mAppData.data(), payload.mAppDataLen);
			qv4 o;
			stringToQuaternion4(msg, o);
			dbglog << "Networked:onNetworkEvent: Quaternion: " << o;

			mOrientation = o;
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
			emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, v, ownerHandle());
			break;
		}
		case ID::PE_UPDATE_ROTATION_VELOCITY:
		{
			assert(ownerHandle() == payload.mAppDestination);

			std::string msg(payload.mAppData.data(), payload.mAppDataLen);
			v3 v;
			stringToVector3(msg, v);
			dbglog << "Networked:onNetworkEvent: RotationVelocity: " << v;
			emit<Physics::Event>(ID::PE_UPDATE_ROTATION_VELOCITY, v, ownerHandle());
			break;
		}


		}
	}
	} // switch e->getId()
}

void Networked::onPhysicsEvent(Physics::Event* e)
{
	switch(e->getId())
	{
		// 		case ID::PE_FULL_SYNC:
		// 			onFullSync(boost::get<Physics::FullSyncData>(e->getData()));
		// 			break;

	case ID::PE_POSITION:
		onPosition(boost::get<v3>(e->getData()));
		break;

	case ID::PE_ORIENTATION:
		onOrientation(boost::get<qv4>(e->getData()));
		break;

	case ID::PE_VELOCITY:
		onVelocity(boost::get<Physics::VelocityComposite>(e->getData()));
		break;

	case ID::PE_ROTATION_VELOCITY:
		onRotationVelocity(boost::get<Physics::VelocityComposite>(e->getData()));
		break;
	default:
		warnlog << "Networked: Can't handle event with ID: "
			<< e->getId();
		break;
	}
}

void Networked::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	if (!receivesData())
	{
		if (mTimer.stop() < 1000)
			return;

		mTimer.restart();

		if (mUpdatePosition)
		{
			const f32 epsilon = 0.1f;
			if (!nearEnough(mInterpolationData.get<2>(), mDeltaStorage.get<0>(), epsilon))
			{

				dbglog << "Networked:onPosition: " << mInterpolationData.get<2>();

				std::stringstream ss;
				ss << mInterpolationData.get<2>();

				CharArray512T ca512 = stringToArray<512>(ss.str());

				BFG::Network::DataPayload payload
				(
					ID::PE_UPDATE_POSITION, 
					ownerHandle(),
					ownerHandle(),
					ss.str().length(),
					ca512
				);

				emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);
				mDeltaStorage.get<0>() = mInterpolationData.get<2>();
			}
			mUpdatePosition = false;
		}

		if (mUpdateOrientation)
		{
			if (angleBetween(mOrientation, mDeltaStorage.get<1>()) < 0.08727f)
				return;

			std::stringstream ss;
			ss << mOrientation;

			CharArray512T ca512 = stringToArray<512>(ss.str());

			BFG::Network::DataPayload payload
			(
				ID::PE_UPDATE_ORIENTATION, 
				ownerHandle(),
				ownerHandle(),
				ss.str().length(),
				ca512
			);

			emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);
			mDeltaStorage.get<1>() = mOrientation;
			mUpdateOrientation = false;
		}

	}
	else
	{
		Location go = getGoValue<Location>(ID::PV_Location, pluginId());

		if (!mInitialized)
		{
			mInterpolationData.get<2>() = go.position;
			mOrientation = go.orientation;
			mInitialized = true;
		}

		if (mUpdatePosition)
		{
			// Only update if the new position is too different from our own calculated one.
			v3 velocity = getGoValue<v3>(ID::PV_Velocity, pluginId());
			f32 speed = length(velocity);

			if (!nearEnough(go.position, mInterpolationData.get<2>(), speed * 0.1f))
			{
				dbglog << "Updating since distance was " << length(go.position - mInterpolationData.get<2>());
				dbglog << "Speed was " << speed;
				emit<Physics::Event>(ID::PE_INTERPOLATE_POSITION, mInterpolationData, ownerHandle());
			}
			mUpdatePosition = false;
		}
		if (mUpdateOrientation)
		{
			if(angleBetween(mOrientation, go.orientation) > 0.08727f)
			{
				dbglog << "AngleBetween: " << angleBetween(mOrientation, go.orientation);
				emit<Physics::Event>(ID::PE_UPDATE_ORIENTATION, mOrientation, ownerHandle());
			}
			mUpdateOrientation = false;
		}
	}

}

void Networked::internalOnEvent(EventIdT action,
                                Property::Value payload,
                                GameHandle module,
                                GameHandle sender)
{
	switch(action)
	{
	case ID::GOE_SYNCHRONIZATION_MODE:
	{
		onSynchronizationMode(static_cast<ID::SynchronizationMode>(static_cast<s32>(payload)));
	}
	}
}

void Networked::onPosition(const v3& newPosition)
{
	if (!sendsData())
		return;

	mInterpolationData = boost::make_tuple(0, 0, newPosition);
	dbglog << "Networked:onPosition(original): " << newPosition;
	mUpdatePosition = true;
}

void Networked::onOrientation(const qv4& newOrientation)
{
	// Don't send if not either WRITE or RW
	if (!sendsData())
		return;

	mOrientation = newOrientation;
	dbglog << "Networked:onOrientation: " << newOrientation;
	mUpdateOrientation = true;
}

void Networked::onVelocity(const Physics::VelocityComposite& newVelocity)
{
	if (!sendsData())
		return;

	dbglog << "Networked:onVelocity: " << newVelocity.get<0>();

	std::stringstream ss;
	ss << newVelocity.get<0>();

	CharArray512T ca512 = stringToArray<512>(ss.str());

	BFG::Network::DataPayload payload
	(
		ID::PE_UPDATE_VELOCITY, 
		ownerHandle(),
		ownerHandle(),
		ss.str().length(),
		ca512
	);

	emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);
}

void Networked::onRotationVelocity(const Physics::VelocityComposite& newVelocity)
{
	if (!sendsData())
		return;

	dbglog << "Networked:onRotationVelocity: " << newVelocity.get<0>();

	std::stringstream ss;
	ss << newVelocity.get<0>();

	CharArray512T ca512 = stringToArray<512>(ss.str());

	BFG::Network::DataPayload payload
	(
		ID::PE_UPDATE_ROTATION_VELOCITY, 
		ownerHandle(),
		ownerHandle(),
		ss.str().length(),
		ca512
	);

	emit<BFG::Network::DataPacketEvent>(BFG::ID::NE_SEND, payload);
}

void Networked::onSynchronizationMode(ID::SynchronizationMode mode)
{
	mSynchronizationMode = mode;

	dbglog << "Networked: setting synchronization mode to " << mode;
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

} // namespace BFG