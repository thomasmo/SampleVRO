#include "stdafx.h"
#include "OpenVRHelper.h"
#include "DrawHelper.h"
#include "SampleVRO.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR lpCmdLine, int nCmdShow)
{
	_RPTF2(_CRT_WARN, "Starting SampleVRO with args \"%S\" at PID 0n%d\n", lpCmdLine, ::GetCurrentProcessId());

	MainWindow win;
	if (wcslen(lpCmdLine) == 0)
	{
		// This is the main process
		_RPTF0(_CRT_WARN, "  Starting SampleVRO main process\n");

		win.CreateChildProcs();
		if (!win.Create(L"SampleVRO", WS_OVERLAPPEDWINDOW, 0, 50, 50, 640, 320))
		{
			return 0;
		}

		ShowWindow(win.Window(), nCmdShow);
	}
	else if (wcscmp(lpCmdLine, CHILD_PROC) == 0)
	{
		_RPTF0(_CRT_WARN, "  Starting SampleVRO child process\n");
	}
	else if (wcscmp(lpCmdLine, GFX_PROC) == 0)
	{
		_RPTF0(_CRT_WARN, "  Starting SampleVRO graphics process\n");
	}
	else
	{
		assert(!"Unsupported arg");
		return 0;
	}

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	_RPTF1(_CRT_WARN, "Closing SampleVRO at PID 0x%d\n", ::GetCurrentProcessId());

	if (wcslen(lpCmdLine) == 0)
	{
		win.TerminateChildProcs();
	}

	return 0;
}