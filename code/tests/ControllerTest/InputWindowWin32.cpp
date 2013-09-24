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

#ifdef _WIN32

#include "InputWindowWin32.h"

#include <stdexcept>

#include "resource.h"

HWND g_win = NULL;

LRESULT DlgProc (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_DESTROY)
		PostQuitMessage(0);
		
	return DefWindowProc(hWnd, Msg, wParam, lParam);
}

HWND createInputGrabbingWindow()
{
	HWND hWnd = CreateDialog(0, MAKEINTRESOURCE(IDD_DIALOG1), 0,(DLGPROC)DlgProc);

	if (! hWnd)
	{
		throw std::runtime_error("Failed to create an input grabbing window!");
	}

	DisableProcessWindowsGhosting();
	ShowWindow(hWnd, SW_SHOW);
	return hWnd;
}

void updateInputGrabbingWindow()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) 
	{ 
		TranslateMessage(&msg); 
		DispatchMessage(&msg); 
	}
}

void destroyInputGrabbingWindow()
{
	DestroyWindow(g_win);
}

#endif // _WIN32
