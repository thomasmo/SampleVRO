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

// Class-wide override to enable calls made to OpenVR
bool OpenVRHelper::s_isEnabled = true;

void OpenVRHelper::Init(HWND hwndMain, HWND hwndOvr, RECT rcHwnd)
{
	if (s_isEnabled)
	{
		m_hwndMain = hwndMain;
		m_rcHwndMain = rcHwnd;
		m_hwndOvr = hwndOvr;

		// COpenVROverlayController::Init
		vr::EVRInitError eError = vr::VRInitError_None;
		m_pHMD = vr::VR_Init(&eError, vr::VRApplication_Overlay);
		if (eError == vr::VRInitError_None)
		{
			m_pHMD->GetDXGIOutputInfo(&m_dxgiAdapterIndex);
			assert(m_dxgiAdapterIndex != -1);
			CreateOverlay();
		}
		else
		{
			assert(!"Failed to initialze OpenVR");
		}
	}
	else
	{
		_RPTF0(_CRT_WARN, "\n\t**OpenVRHelper is disabled for this session.\n");
	}
}


void OpenVRHelper::CreateOverlay()
{
	if (s_isEnabled)
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
				overlayError = vr::VROverlay()->SetOverlayWidthInMeters(m_ulOverlayHandle, 1.5f);
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
								vr::HmdVector2_t vecWindowSize = { (float)m_rcHwndMain.right, (float)m_rcHwndMain.bottom };
								overlayError = vr::VROverlay()->SetOverlayMouseScale(m_ulOverlayHandle, &vecWindowSize);
								if (overlayError == vr::VROverlayError_None)
								{
									vr::VROverlay()->ShowDashboard(rgchKey);

									// Note: bUseMinimalMode set to true so that each char arrives as an event.
									vr::VROverlayError overlayError = vr::VROverlay()->ShowKeyboardForOverlay(
										m_ulOverlayHandle,
										vr::k_EGamepadTextInputModeNormal,
										vr::k_EGamepadTextInputLineModeSingleLine,
										"SampleVRO", // pchDescription,
										100, // unCharMax,
										"", // pchExistingText,
										true, // bUseMinimalMode
										0 //uint64_t uUserValue
									);

									PostVRPollMsg(m_hwndOvr);
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
}

// Post a custom VR_POLL msg for asynchronous processing
void OpenVRHelper::PostVRPollMsg(HWND hwnd)
{
	if (s_isEnabled)
	{
		bool success = PostMessage(hwnd, WM_VR_POLL, 0, 0);
		assert(success || !"Failed to post VR msg");
	}
}

// COpenVROverlayController::OnTimeoutPumpEvents()
void OpenVRHelper::OverlayPump()
{
	assert(s_isEnabled);

	if (vr::VROverlay() != nullptr)
	{
		vr::VREvent_t vrEvent;
		while (vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayHandle, &vrEvent, sizeof(vrEvent)))
		{
			// _RPTF1(_CRT_WARN, "VREvent_t.eventType: %s\n", vr::VRSystem()->GetEventTypeNameFromEnum((vr::EVREventType)(vrEvent.eventType)));
			switch (vrEvent.eventType)
			{
				case vr::VREvent_MouseButtonDown:
				{
					vr::VREvent_Mouse_t data = vrEvent.data.mouse;
					_RPTF1(_CRT_WARN, "VREvent_t.data.mouse (%f, %f)\n", data.x, data.y);
					
					// Windows' origin is top-left, whereas OpenVR's origin is bottom-left, so transform
					// the y-coordinate.
					POINT pt;
					pt.x = (LONG)(data.x);
					pt.y = m_rcHwndMain.bottom - (LONG)(data.y);

					// Route this back to the main window for processing
					PostMessage(m_hwndMain, WM_VRFWD_LBUTTONDOWN, 0, POINTTOPOINTS(pt));
				
					break;
				}
				case vr::VREvent_KeyboardCharInput:
				{
					vr::VREvent_Keyboard_t data = vrEvent.data.keyboard;
					_RPTF1(_CRT_WARN, "  VREvent_t.data.keyboard.cNewInput --%s--\n", data.cNewInput);

					// Route this back to main window for processing
					PostMessage(m_hwndMain, WM_VRFWD_CHAR, data.cNewInput[0], 0);
					break;
				}
			}
		}
	}
}

vr::VROverlayError OpenVRHelper::SetOverlayTexture(vr::VROverlayHandle_t ulOverlayHandle, ID3D11Texture2D* pTex)
{
	if (s_isEnabled)
	{
		vr::Texture_t overlayTextureDX11 = {
			pTex,
			vr::TextureType_DirectX,
			vr::ColorSpace_Gamma
		};

		vr::VROverlayError error = vr::VROverlay()->SetOverlayTexture(ulOverlayHandle, &overlayTextureDX11);
		assert(error == vr::VROverlayError_None);
		return error;
	}
	else
	{
		return vr::VROverlayError_None;
	}
}

// In order to draw from a process that didn't create the overlay, SetOverlayRenderingPid must be called
// for that process to allow it to render to the overlay texture.
void OpenVRHelper::SetDrawPID(DWORD pid)
{
	vr::VROverlayError error = vr::VROverlay()->SetOverlayRenderingPid(m_ulOverlayHandle, pid);
	assert(error == vr::VROverlayError_None);
}