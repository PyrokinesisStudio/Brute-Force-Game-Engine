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

#include <Core/GameHandle.h>

#include <bitset>
#include <cassert>
#include <sstream>

namespace BFG {

typedef BFG::u32 CounterT;

//! From http://stackoverflow.com/questions/3061721/concatenate-boostdynamic-bitset-or-stdbitset
template<std::size_t N1, std::size_t N2>
std::bitset<N1 + N2> concat(const std::bitset<N1> & b1, const std::bitset<N2>& b2)
{
	std::string s1 = b1.to_string();
	std::string s2 = b2.to_string();
	return std::bitset<N1 + N2>(s1 + s2);
}

class HandleGenerator
{
public:
	enum Flags
	{
		NETWORK = 0
	};
	
	static GameHandle createNormal()
	{
		std::bitset<4> flags(0);
		return create(flags);
	}

	static GameHandle createForNetwork()
	{
		std::bitset<4> flags(0);
		flags.set(NETWORK);
		return create(flags);
	}

private:
	static GameHandle create(std::bitset<4> flags)
	{
		++Counter;
		BFG::GameHandle result;
#if _MSC_VER < 1700
		std::bitset<28> counter(static_cast<unsigned long long>(Counter));
#else
		std::bitset<28> counter(Counter);
#endif
		std::bitset<32> handle = concat(counter, flags);
		result = handle.to_ulong();
		return result;
	}

	static CounterT Counter;
};

CounterT HandleGenerator::Counter = 0;

GameHandle generateHandle()
{
	return HandleGenerator::createNormal();
}

GameHandle generateNetworkHandle()
{
	return HandleGenerator::createForNetwork();
}

std::string stringify(GameHandle handle)
{
	std::stringstream ss;
	ss << handle;
	return ss.str();
}

GameHandle destringify(const std::string& name)
{
	assert(! name.empty());

	GameHandle handle;
	std::stringstream ss;
	ss << name;
	ss >> handle;
	return handle;
}

} // namespace BFG

