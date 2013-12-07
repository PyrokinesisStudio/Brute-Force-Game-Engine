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

#ifndef BFG_LEVEL_OF_DETAIL_H_
#define BFG_LEVEL_OF_DETAIL_H_

#include <vector>
#include <stdexcept>
#include <string>

#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/qv4.h>

#include <Core/XmlFileHandleFactory.h>


namespace BFG {


struct Quantifier
{
	Quantifier();
	Quantifier(f32 dtC,
	           f32 foM,
	           f32 soO,
	           f32 vel,
	           f32 direc,
	           f32 atA);
	
	void reload();

	//! quantifier DistanceToCamera
	f32 mDtC;

	//! quantifier FactorOfMobility
	f32 mFoM;

	//! quantifier SizeOfObject
	f32 mSoO;
		
	// Quanitfier for "Factor of Mobility".The sum of all three values must be 1.0f.
	f32 mVelocity;
	f32 mDirection;
	f32 mAtA;

private:

	void load();
	void loadQuantifier(XmlTreeT quantifier, const std::string& name, f32& out);
	void validate();

	std::string mLoDFileName;
};

//! For calculating the LoD value there are several values we consider. Each of these values
//! defines ranges which tell us how likely or unlikely an object can is or can come into the
//! near of the camera. A low range number suggest a higher LoD and a higher number a lower LoD.
struct Ranges
{
	typedef std::pair<u32, f32> RangeDescriptorT;
	typedef std::vector<RangeDescriptorT> RangeTableT;

	Ranges();

	Ranges(RangeTableT dtC,
	       RangeTableT soO,
	       RangeTableT vel,
	       RangeTableT direc,
	       RangeTableT atA);

	u32 velocity(f32 vel) const	 { return range(mVel, vel);	}
	u32 direction(f32 ori) const { return range(mDirec, ori); }
	u32 atA(f32 ata) const       { return range(mAtA, ata); }

	u32 dtC(f32 dtC) const       { return range(mDtC, dtC); }
	u32 soO(f32 soO) const       { return range(mSoO, soO); }

	void reload();

private:
	
	void load();
	void loadRangeTable(XmlTreeT rangeTables, const std::string& tableName, RangeTableT& out);

	//! returns the range number.
	u32 range(const RangeTableT& rangeTable, f32 value) const;

	//! DistanceToCamera
	RangeTableT mDtC;
	//! SizeOfObject
	RangeTableT mSoO;
	
	//! Velocity
	RangeTableT mVel;

	//! Direction (Orientation)
	RangeTableT mDirec;
	
	//! AbilityToAccelerate
	RangeTableT mAtA;

	std::string mLoDFileName;
};
	
	
class LevelOfDetailDefault
{
public:

	u32 get(v3 position, 
		    qv4 orientation, 
		    v3 cameraPosition, 
		    f32 velocitiy, 
		    f32 abilityToAccelerate, 
		    f32 sizeOfObject);

	void reload();

private:

	//! DistanceToCamera
	u32 mDtC;
	//! FactorOfMobility
	f32 mFoM;
	//! SizeOfObject
	u32 mSoO;

	Quantifier mQ;
	Ranges mRanges;
};

} // namespace BFG

#endif
