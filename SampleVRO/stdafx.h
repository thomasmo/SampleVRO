// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

template <class T> void SafeRelease(T **ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = nullptr;
	}
}

#define OVR_PROC	L"--ovr"
#define DRAW_PROC	L"--draw"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// reference additional headers your program requires here
#include <assert.h>

// https://stackoverflow.com/questions/293723/how-could-i-create-a-custom-windows-message
#define WM_VR_POLL				(WM_USER+0)
#define WM_SHARE_DRAW_HWND		(WM_USER+1)
#define WM_SHARE_OVR_HWND		(WM_USER+2)
#define WM_VR_CHAR				(WM_USER+3)
#define WM_VR_POINT				(WM_USER+4)
#define WM_VRFWD_LBUTTONDOWN	(WM_USER+5)
#define WM_VRFWD_CHAR			(WM_USER+6)
#define WM_VR_PAINT				(WM_USER+7)
#define WM_VR_DXADAPTER			(WM_USER+8)