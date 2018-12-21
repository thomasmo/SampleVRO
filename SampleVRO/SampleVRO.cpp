// https://docs.microsoft.com/en-us/windows/desktop/LearnWin32/your-first-direct2d-program
// https://docs.microsoft.com/en-us/windows/desktop/Direct2D/how-to--draw-text
// https://docs.microsoft.com/en-us/windows/desktop/direct2d/direct2d-and-direct3d-interoperation-overview

#include "stdafx.h"
#include "OpenVRHelper.h"
#include "DrawHelper.h"

#include <windows.h>
#include <d2d1.h>
#include <d2d1_1.h>
#include <d3d11_1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "d3d11")



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

	DrawHelper				drawHelper;
	OpenVRHelper			ovrHelper;

	void    OnPaint();
	void    Resize();
	void	SaveChar(WPARAM wParam);

public:

	MainWindow() :
		cchTypeBuffer(0)
	{
	}

	PCWSTR  ClassName() const { return L"Circle Window Class"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Recalculate drawing layout when the size of the window changes.




void MainWindow::OnPaint()
{
	drawHelper.Draw(m_hwnd, &ovrHelper, pchTypeBuffer, cchTypeBuffer);
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
	MainWindow win;
	

	if (!win.Create(L"SampleVRO", WS_OVERLAPPEDWINDOW, 0, 50, 50, 640, 320))
	{
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);	

	// Run the message loop.

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
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
	}
	return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}
