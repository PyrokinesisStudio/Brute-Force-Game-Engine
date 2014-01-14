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

#ifndef BFG_VIEW_SCROLLBOUND_H
#define BFG_VIEW_SCROLLBOUND_H

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include <string>

#include <MyGUI_Gui.h>
#include <MyGUI_ScrollBar.h>

#include <Core/Math.h>
#include <Core/Types.h>


namespace BFG {
namespace View {

struct ScrollBound
{
	typedef boost::function<void(const ScrollBound&, const size_t)> UpdateFct;

	ScrollBound() :
	mValue(0),
	mScrollBar(NULL),
	mRange(0),
	mUpdate(boost::bind(&ScrollBound::noCallback, this, _1, _2))
	{}

	ScrollBound(const size_t value, MyGUI::ScrollBar* _scrollBar = nullptr) :
	mValue(value),
	mScrollBar(_scrollBar),
	mRange(0),
	mUpdate(boost::bind(&ScrollBound::noCallback, this, _1, _2))
	{
		if (mScrollBar)
		{
			mScrollBar->eventScrollChangePosition = MyGUI::newDelegate(this, &ScrollBound::onScrollChanged);
			mRange = mScrollBar->getScrollRange();
		}
	}

	ScrollBound(MyGUI::ScrollBar* _scrollBar) :
	mScrollBar(_scrollBar),
	mUpdate(boost::bind(&ScrollBound::noCallback, this, _1, _2))
	{
		mScrollBar->eventScrollChangePosition = MyGUI::newDelegate(this, &ScrollBound::onScrollChanged);
		mRange = mScrollBar->getScrollRange();
	}

	ScrollBound(const std::string& scrollBarName) :
	mUpdate(boost::bind(&ScrollBound::noCallback, this, _1, _2))
	{
		MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
		mScrollBar = gui->findWidget<MyGUI::ScrollBar>(scrollBarName);
		mScrollBar->eventScrollChangePosition = MyGUI::newDelegate(this, &ScrollBound::onScrollChanged);
		mRange = mScrollBar->getScrollRange();
	}

	operator size_t() const {return mValue;}

	void setWidget(MyGUI::ScrollBar* scrollBar)
	{
		mScrollBar = scrollBar;
		mScrollBar->eventScrollChangePosition = MyGUI::newDelegate(this, &ScrollBound::onScrollChanged);
		mRange = mScrollBar->getScrollRange();
		keepRange();
		synchronizeMyGUI();
	}

	void setCallback(UpdateFct callback)
	{
		mUpdate = callback;
	}

	ScrollBound& operator= (const size_t& _rhs)
	{
		mValue = _rhs;
		keepRange();
		synchronizeMyGUI();

		return *this;
	}

	ScrollBound& operator++()
	{
		++mValue;
		keepRange();
		synchronizeMyGUI();
		return *this;
	}

	ScrollBound& operator--()
	{
		--mValue;
		keepRange();
		synchronizeMyGUI();
		return *this;
	}

	MyGUI::ScrollBar* scrollBar() const
	{
		return mScrollBar;
	}

private:

	void synchronizeMyGUI() const
	{
		if (mScrollBar)
			mScrollBar->setScrollPosition(mValue);
	}

	void keepRange()
	{
		if (mRange)
			mValue = clamp<size_t>(mValue, 0, mRange);
	}

	void onScrollChanged(MyGUI::ScrollBar* _sender, size_t _position)
	{
		mValue = _position;
		mUpdate(*this, mValue);
	}

	void noCallback(const ScrollBound&, const size_t){}
	
	size_t mValue;
	MyGUI::ScrollBar* mScrollBar;

	size_t mRange;
	UpdateFct mUpdate;
};

} // namespace View
} // namespace BFG

#endif
