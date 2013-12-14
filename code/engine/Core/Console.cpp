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

#include <Core/Console.h>

#include <Base/Logger.h>
#include <assert.h>


namespace BFG {

Command::Command(function_ptr fptr) : mFunction(fptr) {}
	
void Command::add(std::deque<std::string> tokens, function_ptr ptr)
{
	auto it = mCommands.find(tokens.front());

	// last token means no further subcommand.
	if (tokens.size() == 1)
	{
		// No double definition of command is allowed.
		assert(it == mCommands.end());
        
		mCommands[tokens[0]] = boost::shared_ptr<Command>(new Command(ptr));
		return;
	}
        
	// No such command already exits. Add a new one and proceed with next subcommand.
	if (it == mCommands.end())
	{
		boost::shared_ptr<Command> tmpCmd(new Command);
		mCommands[tokens.front()] = tmpCmd;
		tokens.pop_front();
		tmpCmd->add(tokens, ptr);
		return;
	}

	// Command already exists. Proceed with next subcommand.
	tokens.pop_front();
	it->second->add(tokens, ptr);
}

void Command::execute(const std::string& commandLine)
{
	std::string out;
	std::string rest;

	// Eat next expression.
	split(commandLine, " ", out, rest);

	// Probably a one expression command.
	if (out.empty())
		out = commandLine;

	auto cmd = mCommands.find(out);

	// Another subcommand found. Go on.
	if (cmd != mCommands.end())
	{
		cmd->second->execute(rest);
		return;
	}
	
	// Expression not found.
	if (!mFunction)
		infolog << "Unknown Command!"; // Dead end.
	else
		mFunction(commandLine); // Good end.
}



void registerCommand(const std::string& command, function_ptr ptr, Command& commands)
{
    std::deque<std::string> tokens;
	tokenize(command, " ", tokens);
    assert(!tokens.empty());
    
    commands.add(tokens, ptr);
}


Console::Console(CommandPtrT commands, std::vector<s32> events, Event::SubLanePtr sublane): 
	mCommands(commands),
	mSubLane(sublane)
{
    for(auto it = events.begin(); it != events.end(); ++it)
		mSubLane->connect(*it, this, &Console::onCommand);
}


void Console::onCommand(const std::string& line)
{
    if (line.empty())
        return;
 
	mCommands->execute(line);
}

} // namespace BFG
