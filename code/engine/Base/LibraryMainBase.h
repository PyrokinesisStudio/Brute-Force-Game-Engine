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

#ifndef BFG_BASE_LIBRARYMAINBASE_H
#define BFG_BASE_LIBRARYMAINBASE_H

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <Base/EntryPoint.h>
#include <Base/ShowException.h>

namespace BFG {
namespace Base {

template <typename FirstArgumentT>
struct LibraryMainBase
{
	virtual ~LibraryMainBase()
	{}
	
	BFG::Base::IEntryPoint* entryPoint()
	{
		return new BFG::Base::CClassEntryPoint<LibraryMainBase>
		(
			this,
			&LibraryMainBase::realMain,
			NULL,
			"Unused Parameter"
		);
	}

protected:
	virtual void main(FirstArgumentT* loop) = 0;

private:
	//! Real entry point for the new thread, called by the EventSystem.
	void* realMain(void* p)
	{
		BOOST_STATIC_ASSERT(( !boost::is_pointer<FirstArgumentT>::value ));
		// TODO: Test Assertion
		assert(p);
		FirstArgumentT* arg = static_cast<FirstArgumentT*>(p);
			
		// Actual main call.
		main(arg);

		// Ignoring the return value of main() on purpose. It gets
		// ignored by the EventSystem, too.
		return 0;
	}
};

} // namespace Base
} // namespace BFG

#endif
