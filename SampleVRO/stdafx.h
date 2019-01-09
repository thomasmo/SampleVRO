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

#define CHILD_PROC	L"--child"
#define GFX_PROC	L"--gfx"

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
#define WM_VRPOLL (WM_USER+0)