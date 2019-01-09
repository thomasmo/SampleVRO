// SampleVRO.cpp
// 
// Sample Win32 app built with D3D11 + D2D for use as a VROverlay with OpenVR
// Also takes keyboard input and draws text on the screen.
//
// Code in this file is based upon the following samples
// - https://docs.microsoft.com/en-us/windows/desktop/LearnWin32/your-first-direct2d-program
//
// This file contains definition of BaseWindow and MainWindow, which register with the OS to receive and
// respond to window messages.

#include "stdafx.h"
#include "OpenVRHelper.h"
#include "DrawHelper.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

#define CHILD_PROC	L"--child"
#define GFX_PROC	L"--gfx"

template <class DERIVED_TYPE> 
class BaseWindow
{
public:
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DERIVED_TYPE *pThis = NULL;

		if (uMsg == WM_NCCREATE)
		{
			CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
			pThis = (DERIVED_TYPE*)pCreate->lpCreateParams;
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);

			pThis->m_hwnd = hwnd;
		}
		else
		{
			pThis = (DERIVED_TYPE*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		}
		if (pThis)
		{
			return pThis->HandleMessage(uMsg, wParam, lParam);
		}
		else
		{
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}
	}

	BaseWindow() : m_hwnd(NULL) { }

	BOOL Create(
		PCWSTR lpWindowName,
		DWORD dwStyle,
		DWORD dwExStyle = 0,
		int x = CW_USEDEFAULT,
		int y = CW_USEDEFAULT,
		int nWidth = CW_USEDEFAULT,
		int nHeight = CW_USEDEFAULT,
		HWND hWndParent = 0,
		HMENU hMenu = 0
	)
	{
		WNDCLASS wc = {0};

		wc.lpfnWndProc   = DERIVED_TYPE::WindowProc;
		wc.hInstance     = GetModuleHandle(NULL);
		wc.lpszClassName = ClassName();

		RegisterClass(&wc);

		m_hwnd = CreateWindowEx(
			dwExStyle, ClassName(), lpWindowName, dwStyle, x, y,
			nWidth, nHeight, hWndParent, hMenu, GetModuleHandle(NULL), this
		);

		return (m_hwnd ? TRUE : FALSE);
	}

	HWND Window() const { return m_hwnd; }

protected:

	virtual PCWSTR  ClassName() const = 0;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;

	HWND m_hwnd;
};

class MainWindow : public BaseWindow<MainWindow>
{
	WCHAR					pchTypeBuffer[100] = { 0 };
	UINT					cchTypeBuffer;
	POINTS					rgPoints[25] = { 0 };
	UINT					cPoints;

	DrawHelper				drawHelper;
	OpenVRHelper			ovrHelper;

	PROCESS_INFORMATION procInfoChild = { 0 };
	PROCESS_INFORMATION procInfoGfx = { 0 };

	void    OnPaint();
	void    Resize();
	void	SaveChar(WPARAM wParam);
	void	SavePoint(WPARAM wParam, LPARAM lParam);

public:
	MainWindow() :
		cchTypeBuffer(0),
		cPoints(0)
	{
	}

	PCWSTR  ClassName() const { return L"Circle Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void	CreateChildProcs();
	void	TerminateChildProcs();
};

void MainWindow::OnPaint()
{
	drawHelper.Draw(m_hwnd, &ovrHelper, pchTypeBuffer, cchTypeBuffer, rgPoints, cPoints);
	ovrHelper.PostVRPollMsg();
}

void MainWindow::Resize()
{
	InvalidateRect(m_hwnd, NULL, FALSE);
}

void MainWindow::SaveChar(WPARAM wParam)
{
	pchTypeBuffer[cchTypeBuffer] = (wchar_t)wParam;
	cchTypeBuffer = (cchTypeBuffer + 1) % ARRAYSIZE(pchTypeBuffer);
}

void MainWindow::SavePoint(WPARAM wParam, LPARAM lParam)
{
	rgPoints[cPoints] = MAKEPOINTS(lParam);
	cPoints = (cPoints + 1) % ARRAYSIZE(rgPoints);
}

// Start up the new processes
void MainWindow::CreateChildProcs()
{
	//DebugBreak();

	WCHAR cmd[MAX_PATH + 50] = { 0 };

	int err = swprintf_s(cmd, ARRAYSIZE(cmd), L"%s %s", GetCommandLine(), CHILD_PROC);
	assert(err > 0);

	STARTUPINFO startupInfoChild = { 0 };
	bool fCreateContentProc = CreateProcess(
		nullptr, // lpApplicationName,
		cmd, 
		nullptr, // lpProcessAttributes,
		nullptr, // lpThreadAttributes,
		TRUE, // bInheritHandles,
		0, // dwCreationFlags,
		nullptr, // lpEnvironment,
		nullptr, // lpCurrentDirectory,
		&startupInfoChild,
		&procInfoChild
	);
	assert(fCreateContentProc);

	err = swprintf_s(cmd, ARRAYSIZE(cmd), L"%s %s", GetCommandLine(), GFX_PROC);
	assert(err > 0);

	STARTUPINFO startupInfoGfx = { 0 };	
	fCreateContentProc = CreateProcess(
		nullptr, // lpApplicationName,
		cmd, 
		nullptr, // lpProcessAttributes,
		nullptr, // lpThreadAttributes,
		TRUE, // bInheritHandles,
		0, // dwCreationFlags,
		nullptr, // lpEnvironment,
		nullptr, // lpCurrentDirectory,
		&startupInfoGfx,
		&procInfoGfx
	);
	assert(fCreateContentProc);
}

// Synchronously terminate the new processes
void MainWindow::TerminateChildProcs()
{
	::TerminateProcess(procInfoChild.hProcess, 0);
	::WaitForSingleObject(procInfoChild.hProcess, 2000);

	::TerminateProcess(procInfoGfx.hProcess, 0);
	::WaitForSingleObject(procInfoGfx.hProcess, 2000);
}

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

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		drawHelper.Setup();
		return 0;

	case WM_DESTROY:
		drawHelper.Shutdown();
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_SIZE:
		Resize();
		return 0;

	case WM_CHAR:
		SaveChar(wParam);
		OnPaint();
		return 0;

	case WM_LBUTTONDOWN:
		SavePoint(wParam, lParam);
		OnPaint();
		return 0;

	case WM_VRPOLL:
		ovrHelper.OverlayPump();
		return 0;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
