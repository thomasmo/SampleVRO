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
#include "SampleVRO.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")


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

