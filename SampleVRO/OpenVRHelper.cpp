// OpenVRHelper.cpp
//
// This file contains code to interact with OpenVR, specifically to set the texture
// used for the VROverlay.
// 
// Code in this file is based upon the following samples
// - https://github.com/ValveSoftware/openvr/tree/master/samples/helloworldoverlay
// - https://github.com/ValveSoftware/openvr/tree/master/samples/hellovr_dx12

#include "stdafx.h"
#include "OpenVRHelper.h"

#include "openvr/headers/openvr.h"
#include <d3d11_1.h>


void OpenVRHelper::Init(HWND hwnd, RECT rcHwnd, int32_t *pnAdapterIndex)
{
	m_hwnd = hwnd;
	m_rcHwnd = rcHwnd;
	
	// COpenVROverlayController::Init
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Overlay );
	if (eError == vr::VRInitError_None)
	{
		m_pHMD->GetDXGIOutputInfo(&m_dxgiAdapterIndex);
		assert(m_dxgiAdapterIndex != -1);
		(*pnAdapterIndex) = m_dxgiAdapterIndex;
	}
	else
	{
		assert(!"Failed to initialze OpenVR");
	}
}


void OpenVRHelper::CreateOverlay(ID3D11Texture2D* pTex)
{
	// COpenVROverlayController::Init
	if (vr::VROverlay() != nullptr)
	{
		std::string sKey = std::string("SampleVRO");
		vr::VROverlayError overlayError = vr::VROverlayError_None;

		overlayError = vr::VROverlay()->CreateDashboardOverlay(
			sKey.c_str(),
			sKey.c_str(),
			&m_ulOverlayHandle,
			&m_ulOverlayThumbnailHandle
		);

		if (overlayError == vr::VROverlayError_None)
		{
			overlayError = vr::VROverlay()->SetOverlayWidthInMeters( m_ulOverlayHandle, 1.5f );
			if (overlayError == vr::VROverlayError_None)
			{
				overlayError = vr::VROverlay()->SetOverlayFlag(m_ulOverlayHandle, vr::VROverlayFlags_VisibleInDashboard, true);
				if (overlayError == vr::VROverlayError_None)
				{
					overlayError = vr::VROverlay()->SetOverlayInputMethod(m_ulOverlayHandle, vr::VROverlayInputMethod_Mouse);
					if (overlayError == vr::VROverlayError_None)
					{
						char rgchKey[vr::k_unVROverlayMaxKeyLength] = { 0 };
						vr::VROverlay()->GetOverlayKey(m_ulOverlayHandle, rgchKey, ARRAYSIZE(rgchKey), &overlayError);
						if (overlayError == vr::VROverlayError_None)
						{
							vr::HmdVector2_t vecWindowSize = { (float)m_rcHwnd.right, (float)m_rcHwnd.bottom};
							overlayError = vr::VROverlay()->SetOverlayMouseScale( m_ulOverlayHandle, &vecWindowSize );
							if (overlayError == vr::VROverlayError_None)
							{
								vr::VROverlay()->ShowDashboard(rgchKey);

								PostVRPollMsg();
							}
						}
					}
				}
			}
		}

		assert(overlayError == vr::VROverlayError_None);
	}
	else
	{
		assert(!"Failed to get VROverlay");
	}
}

// Post a custom VRPOLL msg for asynchronous processing
void OpenVRHelper::PostVRPollMsg()
{
	bool success = PostMessage(m_hwnd, WM_VRPOLL, 0, 0);
	assert(success || !"Failed to post VR msg");
}

// COpenVROverlayController::OnTimeoutPumpEvents()
void OpenVRHelper::OverlayPump()
{
	if (vr::VROverlay() != nullptr)
	{
		vr::VREvent_t vrEvent;
		while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayHandle, &vrEvent, sizeof(vrEvent)))
		{
			switch (vrEvent.eventType)
			{
				case vr::VREvent_MouseButtonDown:
				{
					vr::VREvent_Mouse_t data = vrEvent.data.mouse;
					_RPTF1(_CRT_WARN, "VREvent_t.data.mouse (%f, %f)\n", data.x, data.y);
					
					// Windows' origin is top-left, whereas OpenVR's origin is bottom-left, so transform
					// the y-coordinate.
					POINT pt;
					pt.x = data.x;
					pt.y = m_rcHwnd.bottom - data.y;

					// Route this back to the main window for processing
					PostMessage(m_hwnd, WM_LBUTTONDOWN, 0, POINTTOPOINTS(pt));
				
					break;
				}
			}
		}
	}
}

vr::VROverlayError OpenVRHelper::SetOverlayTexture(ID3D11Texture2D* pTex)
{
	vr::Texture_t overlayTextureDX11 = {
		pTex,
		vr::TextureType_DirectX,
		vr::ColorSpace_Gamma
	};

	return vr::VROverlay()->SetOverlayTexture(m_ulOverlayHandle, &overlayTextureDX11);
}