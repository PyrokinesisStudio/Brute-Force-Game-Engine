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

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/qv4.h>
#include <Core/Math.h>
#include <Core/Path.h>
#include <Core/XmlFileHandleFactory.h>


namespace BFG {

struct Quantifier
{
	Quantifier()
	{
		Path p;
		mLoDFileName = p.Get(ID::P_SCRIPTS_SETTINGS) + "/LevelOfDetail.xml";
		
		load();
		validate();
	}
	
	Quantifier(f32 dtC,
	           f32 foM,
	           f32 soO,
	           f32 vel,
	           f32 direc,
	           f32 atA):
	mDtC(dtC),
	mFoM(foM),
	mSoO(soO),
	mVelocity(vel),
	mDirection(direc),
	mAtA(atA)
	{
		validate();
	}

	void reload()
	{
		load();
	}


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

	void load()
	{
		XmlFileHandleT levelOfDetailXml = createXmlFileHandle(mLoDFileName);
		XmlTreeT quantifier = levelOfDetailXml->root()->child("Settings")->child("LevelOfDetail")->child("Quantifier");

		loadQuantifier(quantifier, "DistanceToCamera", mDtC);
		loadQuantifier(quantifier, "FactorOfMobility", mFoM);
		loadQuantifier(quantifier, "SizeOfObject", mSoO);
		loadQuantifier(quantifier, "Velocity", mVelocity);
		loadQuantifier(quantifier, "Direction", mDirection);
		loadQuantifier(quantifier, "AbilityToAccelerate", mAtA);
	}

	void loadQuantifier(XmlTreeT quantifier, const std::string& name, f32& out)
	{
		std::string value =  quantifier->child(name)->elementData();
		out = boost::lexical_cast<f32>(value);
	}

	void validate()
	{
		if (!nearEnough(mDtC + mFoM + mSoO, 1.0f, EPSILON_F) || 
		    !nearEnough(mVelocity + mDirection + mAtA, 1.0f, EPSILON_F))
		{
			throw std::logic_error("Invalid LoD quantifier. DtC + FoM + SoO must be 1.0f and vel + direct + AtA must be 1.0f, too.");
		}
	}

	std::string mLoDFileName;
};

//! For calculating the LoD value there are several values we consider. Each of these values
//! defines ranges which tell us how likely or unlikely an object can is or can come into the
//! near of the camera. A low range number suggest a higher LoD and a higher number a lower LoD.
struct Ranges
{
	typedef std::pair<u32, f32> RangeDescriptorT;
	typedef std::vector<RangeDescriptorT> RangeTableT;

	Ranges()
	{
		Path p;
		mLoDFileName = p.Get(ID::P_SCRIPTS_SETTINGS) + "/LevelOfDetail.xml";
		load();
	}

	Ranges(RangeTableT dtC,
	       RangeTableT soO,
	       RangeTableT vel,
	       RangeTableT direc,
	       RangeTableT atA):
	mDtC(dtC),
	mSoO(soO),
	mVel(vel),
	mDirec(direc),
	mAtA(atA)
	{
	}

	u32 velocity(f32 vel) const	 { return range(mVel, vel);	}
	u32 direction(f32 ori) const { return range(mDirec, ori); }
	u32 atA(f32 ata) const       { return range(mAtA, ata); }

	u32 dtC(f32 dtC) const       { return range(mDtC, dtC); }
	u32 soO(f32 soO) const       { return range(mSoO, soO); }

	void reload()
	{
		load();
	}

private:
	
	void load()
	{
		XmlFileHandleT levelOfDetailXml = createXmlFileHandle(mLoDFileName);
		XmlTreeT rangeTables = levelOfDetailXml->root()->child("Settings")->child("LevelOfDetail")->child("RangeTables");

		loadRangeTable(rangeTables, "DistanceToCamera", mDtC);
		loadRangeTable(rangeTables, "SizeOfObject", mSoO);
		loadRangeTable(rangeTables, "Velocity", mVel);
		loadRangeTable(rangeTables, "Direction", mDirec);
		loadRangeTable(rangeTables, "AbilityToAccelerate", mAtA);
	}

	void loadRangeTable(XmlTreeT rangeTables, const std::string& tableName, RangeTableT& out)
	{
		XmlTreeListT rangeDescriptors = rangeTables->child(tableName)->childList("RangeDescriptor");
		
		std::string rating;
		std::string value;

		BOOST_FOREACH(XmlTreeT tree, rangeDescriptors)
		{
			rating = tree->child("rating")->elementData();
			value = tree->child("value")->elementData();
			RangeDescriptorT descr(boost::lexical_cast<u32>(rating), boost::lexical_cast<f32>(value));
			out.push_back(descr);
		}
	}

	//! returns the range number.
	u32 range(const RangeTableT& rangeTable, f32 value) const
	{
		if (rangeTable.empty())
			throw std::runtime_error("No ranges have been set. The vetors are empty.");
		
		RangeTableT::const_iterator lhRange = rangeTable.begin();
		RangeTableT::const_iterator rhRange = rangeTable.begin();
		++rhRange;

		for (; rhRange != rangeTable.end(); ++lhRange, ++rhRange)
		{
			if ((value >= lhRange->second && value < rhRange->second) ||
				(value <= lhRange->second && value > rhRange->second))

				return lhRange->first;
		}
	
		// The value is not between any ranges. So we have to look if we need the upper or lower end.
		// We never know in which directions the ranges are defined (low to hight or high to low numbers).

		bool largeValueFirst = rangeTable.begin()->second > rangeTable.back().second;

		if (largeValueFirst)
		{
			if ((value > rangeTable.begin()->second) && (value > rangeTable.back().second))
				return rangeTable.begin()->first;
			else
				return rangeTable.back().first;
		}

		if ((value > rangeTable.begin()->second) && (value > rangeTable.back().second))
			return rangeTable.back().first;
		else
			return rangeTable.begin()->first;	
	}

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
	
	
class LevelOfDetail
{
public:
		LevelOfDetail(const Quantifier& quantifier,
		              const Ranges& ranges):
		mQ(quantifier),
		mRanges(ranges)
		{
		}

		f32 get(v3 position, 
		        qv4 orientation, 
		        v3 cameraPosition, 
		        f32 velocitiy, 
		        f32 abilityToAccelerate, 
		        f32 sizeOfObject)
		{
			v3 directionCam = position - cameraPosition;
			norm(directionCam);

			f32 angle = angleBetween(orientation, eulerToQuaternion(directionCam)) * RAD2DEG;
			
			mFoM = mRanges.velocity(velocitiy) * mQ.mVelocity + 
			       mRanges.direction(angle) * mQ.mDirection + 
			       mRanges.atA(abilityToAccelerate) * mQ.mAtA;

			mSoO = mRanges.soO(sizeOfObject);
			mDtC = mRanges.dtC(distance(position, cameraPosition));
		
			return mDtC * mQ.mDtC + mFoM * mQ.mFoM + mSoO * mQ.mSoO;
		}

private:

		//! DistanceToCamera
		f32 mDtC;
		//! FactorOfMobility
		f32 mFoM;
		//! SizeOfObject
		f32 mSoO;

		const Quantifier& mQ;
		const Ranges& mRanges;
};

} // namespace BFG

#endif
