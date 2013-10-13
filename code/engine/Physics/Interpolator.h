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

#ifndef BFG_INTERPOLATOR_H__
#define BFG_INTERPOLATOR_H__

#include <Base/Interpolate.h>

#include <Core/Math.h>
#include <Core/qv4.h>
#include <Core/Types.h>
#include <Core/v3.h>

namespace BFG {
namespace Physics {

struct Interpolator
{
	Interpolator() :
	mStartOrientation(qv4::IDENTITY),
	mEndOrientation(qv4::IDENTITY),
	mInterpolatedOrientation(qv4::IDENTITY),
	mStartPosition(v3::ZERO),
	mEndPosition(v3::ZERO),
	mInterpolatedPosition(v3::ZERO),
	mOrientationParameter(0.0f),
	mPositionParameter(0.0f),
	mInterpolateOrientation(false),
	mInterpolatePosition(false)
	{}

	void preparePosition(const InterpolationDataV3& interpData, const v3& startPosition, const v3& velocity)
	{
		mInterpolatePosition = true;

		u32 timeStamp = interpData.get<0>(); // TODO: Unused parameter
		u16 age = interpData.get<1>();
		const v3& pos = interpData.get<2>();

		v3 deltaVelocity = velocity * static_cast<f32>(age) / 1000.0f;
		dbglog << "dv/dt: " << deltaVelocity;

		v3 estimatedPosition = pos + deltaVelocity;

		mEndPosition = estimatedPosition;
		mStartPosition = startPosition;
		mPositionParameter = 0.0f;

		dbglog << "Interpolating from " << startPosition
		       << " over last " << pos
		       << " to estimated: " << estimatedPosition
		       << " (" << static_cast<f32>(age)/1000.0f << "s)";
	}

	void prepareOrientation(const InterpolationDataQv4& interpData, const qv4& startOrientation, const v3& rotationVelocity)
	{
		mInterpolateOrientation = true;

		u32 timeStamp = interpData.get<0>();
		u16 age = interpData.get<1>();
		const qv4& ori = interpData.get<2>();

		qv4 estimatedOrientation = ori * eulerToQuaternion(rotationVelocity * static_cast<f32>(age) / 1000.0f);

		mEndOrientation = estimatedOrientation;
		mStartOrientation = startOrientation;
		mOrientationParameter = 0.0f;

		dbglog << "Interpolating from " << startOrientation << " over " << ori << " to " << estimatedOrientation;
	}

	bool interpolatePosition(f32 timeSinceLastFrame)
	{
		if (mInterpolatePosition)
		{
			// Depends on UPDATES_PER_SECOND in Networked::internalUpdate
			// This is UPDATE_DELAY / 1000
			const f32 DURATION = 1.0f;

			// We interpolate from 0.0f to 1.0f
			const f32 SCALE = 1.0f;

			const f32 STEP = timeSinceLastFrame / DURATION;

			mPositionParameter += STEP;
			mInterpolatedPosition = BFG::interpolate
			(
				mStartPosition,
				mEndPosition,
				mPositionParameter
			);
		
			if (mPositionParameter >= SCALE)
				mInterpolatePosition = false;

			return true;
		}
		return false;
	}

	bool interpolateOrientation(f32 timeSinceLastFrame)
	{
		if (mInterpolateOrientation)
		{
			// Depends on UPDATES_PER_SECOND in Networked::internalUpdate
			// This is UPDATE_DELAY / 1000
			const f32 DURATION = 1.0f;

			// We interpolate from 0.0f to 1.0f
			const f32 SCALE = 1.0f;

			const f32 STEP = timeSinceLastFrame / DURATION;

			mOrientationParameter += STEP;

 			mInterpolatedOrientation = lerp 
			(
				mStartOrientation, 
 				mEndOrientation,
				mOrientationParameter
			);

			if (mOrientationParameter >= SCALE)
				mInterpolateOrientation = false;

			return true;
		}
		return false;
	}

	qv4 mStartOrientation;
	qv4 mEndOrientation;
	qv4 mInterpolatedOrientation;

	v3 mStartPosition;
	v3 mEndPosition;
	v3 mInterpolatedPosition;
	
	f32 mOrientationParameter;
	f32 mPositionParameter;

	bool mInterpolateOrientation;
	bool mInterpolatePosition;


};

} // namespace Physics
} // namespace BFG

#endif
