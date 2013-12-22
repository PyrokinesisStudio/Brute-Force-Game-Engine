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

#ifndef BFG_VIEW_TEXTBOUND_H
#define BFG_VIEW_TEXTBOUND_H

#include <string>

#include <MyGUI_Gui.h>
#include <MyGUI_TextBox.h>
#include <Core/Types.h>


namespace BFG {
namespace View {

template <typename T>
struct TextBound : std::ios
{
	TextBound() :
	std::ios(NULL),
	mTextBox(NULL)
	{}

	TextBound(MyGUI::TextBox* _textBox) :
	std::ios(NULL),
	mTextBox(_textBox)
	{}

	TextBound(const std::string& textBoxName) :
	std::ios(NULL)
	{
		MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
		mTextBox = gui->findWidget<MyGUI::TextBox>(textBoxName);
	}

	operator T() const {return value;}

	TextBound<T>& operator= (const T& _rhs)
	{
		value = _rhs;
		synchronizeTextBox();

		return *this;
	}

	TextBound<T>& operator++()
	{
		++value;
		synchronizeTextBox();
		return *this;
	}

	TextBound<T>& operator--()
	{
		--value;
		synchronizeTextBox();
		return *this;
	}

	MyGUI::TextBox* textBox() const
	{
		return mTextBox;
	}

	void preFix(const std::string& preFix)
	{
		mPreFix = preFix;
	}

	void postFix(const std::string& postFix)
	{
		mPostFix = postFix;
	}

private:

	void synchronizeTextBox() const
	{
		if (!mTextBox)
			throw std::runtime_error("TextBox in TextBound was not set!");

		std::stringstream ss;
		ss << mPreFix;
		ss.copyfmt(*this);
		ss << value;
		ss << mPostFix;
		mTextBox->setCaption(ss.str());
	}

	T value;
	MyGUI::TextBox* mTextBox;

	std::string mPreFix;
	std::string mPostFix;
};

} // namespace View
} // namespace BFG

#endif
