#include "stdafx.h"
#include "OpenVRHelper.h"
#include "DrawHelper.h"
#include "SampleVRO.h"
#include <shellapi.h>

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow)
{
	_RPTF2(_CRT_WARN, "Starting SampleVRO with args \"%S\" at PID 0n%d\n", lpCmdLine, ::GetCurrentProcessId());

	int nArgs;
	LPWSTR *szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (szArglist == nullptr)
	{
		assert(!"Invalid Args!");
		return 0;
	}
	
	if (nArgs == 1)
	{
		// This is the main process
		return MainWindow::wWinMain(nCmdShow);
	}
	else if (nArgs == 3)
	{
		wchar_t* pszHwnd = szArglist[2];
		HWND hwndMain = (HWND)wcstoull(pszHwnd, &(pszHwnd) + wcslen(pszHwnd), 16);

		if (wcscmp(szArglist[1], OVR_PROC) == 0)
		{
			return OpenVRWindow::wWinMain(nCmdShow, hwndMain);
		}
		else if (wcscmp(szArglist[1], DRAW_PROC) == 0)
		{	
			return DrawWindow::wWinMain(nCmdShow, hwndMain);
		}
		else
		{
			assert(!"What process is this?");
		}
	}
	else
	{
		assert(!"Unsupported arg");
		return 0;
	}
	
	return 0;
}

void Pump()
{
	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	_RPTF1(_CRT_WARN, "Closing SampleVRO at PID 0x%d\n", ::GetCurrentProcessId());
}

int MainWindow::wWinMain(int nCmdShow)
{
	_RPTF0(_CRT_WARN, "  Starting SampleVRO main process\n");

	MainWindow win;
	if (!win.Create(L"SampleVRO", WS_OVERLAPPEDWINDOW, 0, 50, 50, 640, 320))
	{
		return 0;
	}

	win.CreateChildProcs();
	ShowWindow(win.Window(), nCmdShow);

	Pump();

	win.TerminateChildProcs();
	
	return 0;
}

int DrawWindow::wWinMain(int nCmdShow, HWND hwndMain)
{
	_RPTF0(_CRT_WARN, "  Starting SampleVRO graphics process\n");

	DrawWindow win(hwndMain);
	if (!win.Create(L"SampleVRO--DRAW", WS_OVERLAPPEDWINDOW, 0, 50, 50, 640, 320))
	{
		return 0;
	}

	SendMessage(hwndMain, WM_SHARE_DRAW_HWND, (WPARAM)win.Window(), 0);

	Pump();

	return 0;
}

int OpenVRWindow::wWinMain(int nCmdShow, HWND hwndMain)
{
	_RPTF0(_CRT_WARN, "  Starting SampleVRO OpenVR process\n");

	OpenVRWindow win(hwndMain);
	if (!win.Create(L"SampleVRO--OVR", WS_OVERLAPPEDWINDOW, 0, 50, 50, 640, 320))
	{
		return 0;
	}

	win.OnCreate();

	SendMessage(hwndMain, WM_SHARE_OVR_HWND, (WPARAM)win.Window(), 0);

	Pump();

	return 0;
}