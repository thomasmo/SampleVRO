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
//
// This sample uses 3 processes:
// - Main: Interacts with OS (takes input messages, creates other processes)
// - OVR: Interacts with OpenVR APIs, forwards appropriate messages to Main and Draw processes
// - Draw: Interacts with DirectX, stores content to draw
//
// The bootstrapping of the processes are as follows:
// 
//            Main                      Draw                     OVR
//              |
//       Create other procs--------------+------------------------|
//              |                        |                        |
//              |<--------------  Send Draw HWND            Init OpenVR system
//              |                        |                        |
//              |<----------------------------------------- Send OVR HWND
//              |                        |                        |
//		 Share other HWND  ------------->|----------------------->|
//              |                        |                        |
//              |                        |<---------------- Send DX+OVR info
//              |                        |                        |
//              |                 Init DX resources               |
//              |                        |                        |
//              |                      Draw                       |
//              |                        |                        |
//     Receive OS key/mouse ----> Render content from      Receive OVR key/button
//     input msgs                 input to overlay         input msgs (forward to Main)
//             ^--------------------------------------------------|
//

#include "stdafx.h"
#include "OpenVRHelper.h"
#include "DrawHelper.h"
#include "SampleVRO.h"

/////////////////////////// MainWindow class for Main Process ///////////////////////////

// Forward paint message from OS to Draw process
void MainWindow::OnPaint()
{
	if (hwndDraw != nullptr)
	{
		SendMessage(hwndDraw, WM_VR_PAINT, 0, 0);
	}
}

void MainWindow::Resize()
{
	InvalidateRect(m_hwnd, NULL, FALSE);
}

// Forward key message from OS to Draw process
void MainWindow::SaveChar(WPARAM wParam)
{
	assert(hwndDraw != nullptr);
	SendMessage(hwndDraw, WM_VR_CHAR, wParam, 0);
}

// Forward click message from OS to Draw process
void MainWindow::SavePoint(WPARAM wParam, LPARAM lParam)
{
	assert(hwndDraw != nullptr);
	SendMessage(hwndDraw, WM_VR_POINT, wParam, lParam);
}

// Synchronously start up the new processes
void MainWindow::CreateChildProcs()
{
	//DebugBreak();

	WCHAR cmd[MAX_PATH + 50] = { 0 };

	int err = swprintf_s(cmd, ARRAYSIZE(cmd), L"%s %s 0x%p", GetCommandLine(), OVR_PROC, Window());
	assert(err > 0);

	STARTUPINFO startupInfoOVR = { 0 };
	bool fCreateContentProc = ::CreateProcess(
		nullptr, // lpApplicationName,
		cmd, 
		nullptr, // lpProcessAttributes,
		nullptr, // lpThreadAttributes,
		TRUE, // bInheritHandles,
		0, // dwCreationFlags,
		nullptr, // lpEnvironment,
		nullptr, // lpCurrentDirectory,
		&startupInfoOVR,
		&procInfoOVR
	);
	assert(fCreateContentProc);
	::WaitForInputIdle(procInfoOVR.hProcess, INFINITE);

	err = swprintf_s(cmd, ARRAYSIZE(cmd), L"%s %s 0x%p", GetCommandLine(), DRAW_PROC, Window());
	assert(err > 0);

	STARTUPINFO startupInfoDraw = { 0 };	
	fCreateContentProc = ::CreateProcess(
		nullptr, // lpApplicationName,
		cmd, 
		nullptr, // lpProcessAttributes,
		nullptr, // lpThreadAttributes,
		TRUE, // bInheritHandles,
		0, // dwCreationFlags,
		nullptr, // lpEnvironment,
		nullptr, // lpCurrentDirectory,
		&startupInfoDraw,
		&procInfoDraw
	);
	assert(fCreateContentProc);
	::WaitForInputIdle(procInfoDraw.hProcess, INFINITE);
}

// Synchronously terminate the new processes
void MainWindow::TerminateChildProcs()
{
	::TerminateProcess(procInfoOVR.hProcess, 0);
	::WaitForSingleObject(procInfoOVR.hProcess, 2000);

	::TerminateProcess(procInfoDraw.hProcess, 0);
	::WaitForSingleObject(procInfoDraw.hProcess, 2000);
}

void MainWindow::ShareHwnds()
{
	if (hwndDraw != nullptr && hwndOVR != nullptr)
	{
		SendMessage(hwndDraw, WM_SHARE_OVR_HWND, (WPARAM)hwndOVR, 0);
		SendMessage(hwndOVR, WM_SHARE_DRAW_HWND, (WPARAM)hwndDraw, 0);
	}
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
		OnPaint();
		return 0;

	case WM_SIZE:
		Resize();
		return 0;

	case WM_CHAR:
	case WM_VRFWD_CHAR:
		SaveChar(wParam);
		OnPaint();
		return 0;

	case WM_LBUTTONDOWN:
	case WM_VRFWD_LBUTTONDOWN:
		SavePoint(wParam, lParam);
		OnPaint();
		return 0;

	case WM_SHARE_DRAW_HWND:
		hwndDraw = (HWND)wParam;
		ShareHwnds();
		return 0;

	case WM_SHARE_OVR_HWND:
		hwndOVR = (HWND)wParam;
		ShareHwnds();
		return 0;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

/////////////////////////// DrawWindow class for Draw Process ///////////////////////////

void DrawWindow::OnPaint()
{
	drawHelper.Draw(hwndMain, Window(), pchTypeBuffer, cchTypeBuffer, rgPoints, cPoints);
	OpenVRHelper::PostVRPollMsg(hwndOVR);
}

void DrawWindow::SaveChar(WPARAM wParam)
{
	pchTypeBuffer[cchTypeBuffer] = (wchar_t)wParam;
	cchTypeBuffer = (cchTypeBuffer + 1) % ARRAYSIZE(pchTypeBuffer);
}

void DrawWindow::SavePoint(WPARAM wParam, LPARAM lParam)
{
	rgPoints[cPoints] = MAKEPOINTS(lParam);
	cPoints = (cPoints + 1) % ARRAYSIZE(rgPoints);
}

LRESULT DrawWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_VR_DXADAPTER:
		drawHelper.Setup(
			wParam, // dxgiAdapterIndex
			lParam  // overlayHandle
		);
		return 0;

	case WM_DESTROY:
		drawHelper.Shutdown();
		PostQuitMessage(0);
		return 0;

	case WM_SHARE_OVR_HWND:
		hwndOVR = (HWND)wParam;
		return 0;

	case WM_VR_PAINT:
		OnPaint();
		return 0;

	case WM_VR_CHAR:
		SaveChar(wParam);
		OnPaint();
		return 0;

	case WM_VR_POINT:
		SavePoint(wParam, lParam);
		OnPaint();
		return 0;

	case WM_VR_POLL:
		DebugBreak();
		return 0;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

/////////////////////////// OpenVRWindow class for OVR Process ///////////////////////////

void OpenVRWindow::OnCreate()
{
	RECT rc;
	GetClientRect(hwndMain, &rc);
	ovrHelper.Init(hwndMain, Window(), rc);
}

LRESULT OpenVRWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:	
		PostQuitMessage(0);
		return 0;

	case WM_SHARE_DRAW_HWND:
		hwndDraw = (HWND)wParam;
		DWORD pid;
		GetWindowThreadProcessId(hwndDraw, &pid);
		ovrHelper.SetDrawPID(pid);
		SendMessage(hwndDraw, WM_VR_DXADAPTER, ovrHelper.GetAdapterIndex(), ovrHelper.GetOverlayHandle());
		return 0;

	case WM_VR_POLL:
		ovrHelper.OverlayPump();
		return 0;
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}