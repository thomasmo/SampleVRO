#pragma once

#include "resource.h"

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
	PROCESS_INFORMATION procInfoOVR = { 0 };
	PROCESS_INFORMATION procInfoDraw = { 0 };
	HWND hwndOVR;
	HWND hwndDraw;

	void    OnPaint();
	void    Resize();
	void	SaveChar(WPARAM wParam);
	void	SavePoint(WPARAM wParam, LPARAM lParam);
	void	ShareHwnds();

public:
	MainWindow() :
		hwndOVR(nullptr),
		hwndDraw(nullptr)
	{
	}

	PCWSTR  ClassName() const { return L"SampleVRO--Main"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	void	CreateChildProcs();
	void	TerminateChildProcs();

	static int wWinMain(int nCmdShow);
};

class DrawWindow : public BaseWindow<DrawWindow>
{
	WCHAR					pchTypeBuffer[100] = { 0 };
	UINT					cchTypeBuffer;
	POINTS					rgPoints[25] = { 0 };
	UINT					cPoints;

	DrawHelper				drawHelper;

	HWND					hwndMain;
	HWND					hwndOVR;

	void    OnPaint();
	void	SaveChar(WPARAM wParam);
	void	SavePoint(WPARAM wParam, LPARAM lParam);

public:
	DrawWindow(HWND hwndMainParam) :
		cchTypeBuffer(0),
		cPoints(0),
		hwndMain(hwndMainParam),
		hwndOVR(nullptr)
	{
	}

	PCWSTR  ClassName() const { return L"SampleVRO--Draw"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int wWinMain(int nCmdShow, HWND hwndMain);
};

class OpenVRWindow : public BaseWindow<DrawWindow>
{
	OpenVRHelper			ovrHelper;
	HWND					hwndMain;
	HWND					hwndDraw;

	void    OnPaint();
	void	OnCreate();

public:
	OpenVRWindow(HWND hwndMainParam) :
		hwndMain(hwndMainParam),
		hwndDraw(nullptr)
	{
	}

	PCWSTR  ClassName() const { return L"SampleVRO--OVR"; }
	LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	static int wWinMain(int nCmdShow, HWND hwndMain);
};