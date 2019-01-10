#pragma once

#include "openvr/headers/openvr.h"

struct ID3D11Texture2D;
struct ID3D11DeviceContext;

class OpenVRHelper
{
public:
	OpenVRHelper() :
		m_pHMD(nullptr),
		m_dxgiAdapterIndex(-1),
		m_ulOverlayHandle(vr::k_ulOverlayHandleInvalid),
		m_ulOverlayThumbnailHandle(vr::k_ulOverlayHandleInvalid),
		m_hwndMain(nullptr),
		m_rcHwndMain(),
		m_hwndOvr(nullptr),
		m_pidDraw(-1)
	{
	}
	~OpenVRHelper()
	{
	}

	void Init(HWND hwndMain, HWND hwndOvr, RECT rc);
	void CreateOverlay();

	void SetDrawPID(DWORD pid);
	static vr::VROverlayError SetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle, ID3D11Texture2D* pTex);
	
	static void PostVRPollMsg(HWND hwnd);
	void OverlayPump();

	int32_t GetAdapterIndex() const { return m_dxgiAdapterIndex; }
	vr::VROverlayHandle_t GetOverlayHandle() const { return m_ulOverlayHandle; }

private:
	vr::IVRSystem * m_pHMD;
	int32_t m_dxgiAdapterIndex;
	vr::VROverlayHandle_t m_ulOverlayHandle;
	vr::VROverlayHandle_t m_ulOverlayThumbnailHandle;
	HWND m_hwndMain;
	RECT m_rcHwndMain;
	HWND m_hwndOvr;
	DWORD m_pidDraw;
	static bool s_isEnabled;
};

