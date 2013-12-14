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

#ifndef BFG_CONSOLE_H__
#define BFG_CONSOLE_H__

#include <string>
#include <map>
#include <deque>

#include <boost/shared_ptr.hpp>

#include <Core/String.h>
#include <Event/Event.h>


namespace BFG {

typedef void (*function_ptr)(const std::string&);

struct Command;
typedef boost::shared_ptr<Command> CommandPtrT;
typedef std::map<std::string, CommandPtrT > CommandsT;

//! \brief Recursive structure for console commands.
struct BFG_CORE_API Command
{
	Command(function_ptr fptr = nullptr);
	
	void add(std::deque<std::string> tokens, function_ptr ptr);
	void execute(const std::string& commandLine);

	//! A console command function.
	function_ptr mFunction;
	
	CommandsT mCommands;
};


//! \brief Helper function to define commands.
//! Syntax: registerCommand("Com1 Com2 Com3", &Com3, commands);
void BFG_CORE_API registerCommand(const std::string& command, function_ptr ptr, Command& commands);


//! \brief Receives events for and connect them with a particular command function.
class BFG_CORE_API Console
{
public:
	Console(CommandPtrT commands, std::vector<s32> events, Event::SubLanePtr sublane);

private:

	void onCommand(const std::string& line);
	
	CommandPtrT mCommands;
	Event::SubLanePtr mSubLane;
};

} // namespace BFG

#endif
