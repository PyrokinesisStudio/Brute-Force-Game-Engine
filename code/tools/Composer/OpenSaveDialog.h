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

#ifndef OPEN_SAVE_FILE_DIALOG_H
#define OPEN_SAVE_FILE_DIALOG_H

#include <MyGUI.h>

struct FileInfo
{
	FileInfo(const std::string& name, const std::string& folder) :
	mFileName(name),
	mFolder(folder)
	{}
	
	bool operator < (const FileInfo& rhs) const;

	std::string mFileName;
	std::string mFolder;
};

typedef std::vector<FileInfo> FileInfoVectorT;

class OpenSaveDialog
{
public:
	typedef MyGUI::delegates::CDelegate1<bool> EventHandle_Result;

public:
	OpenSaveDialog(bool sort = true);

	void setDialogInfo(const std::string& caption,
	                   const std::string& button,
	                   MyGUI::delegates::IDelegate1<MyGUI::Widget*>* clickHandler);

	const std::string& getCurrentFolder() {return mCurrentFolder;}

	const std::string& getFileName() const {return mFileName;}

	void setRestrictions(const std::string& startFolder,
	                     bool onlyThisFolder,
	                     const std::string& extension);

	void clearRestrictions();

	void setVisible(bool value)
	{
		mWindow->setVisible(value);
		if (value)
			update();
	}
	bool isVisible() {return mWindow->getVisible();}

	EventHandle_Result eventEndDialog;

private:
	void fillInfoVector(FileInfoVectorT& folders,
	                    FileInfoVectorT& files);

	void notifyListChangePosition(MyGUI::ListBox* sender, size_t index);
	void notifyListSelectAccept(MyGUI::ListBox* sender, size_t index);
	void notifyEditSelectAccept(MyGUI::EditBox* sender);
	void notifyEditTextChanged(MyGUI::EditBox* sender);
	void notifyWindowButtonPressed(MyGUI::Window* sender, const std::string& name);

	void update();
	void accept();

	MyGUI::ListBox* mFilesList;
	MyGUI::EditBox* mFileNameEdit;
	MyGUI::EditBox* mCurrentFolderEdit;
	MyGUI::Button* mOpenSaveButton;
	MyGUI::Window* mWindow;
	MyGUI::VectorWidgetPtr mContainer;

	std::string mExtension;
	bool mOnlyThisFolder;
	bool mSort;

	std::string mCurrentFolder;
	std::string mFileName;
};

#endif
