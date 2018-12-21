#include "stdafx.h"
#include "OpenVRHelper.h"

#include "openvr/headers/openvr.h"
#include <d3d11_1.h>
#include <wincodec.h>
#include "dxtk/Inc/ScreenGrab.h"


void OpenVRHelper::Save(ID3D11DeviceContext* pContext, ID3D11Texture2D* pTex)
{
	// https://github.com/Microsoft/DirectXTK
	DirectX::SaveWICTextureToFile(
		pContext,
		pTex,
		GUID_ContainerFormatBmp,
		L"SampleVRO.bmp"
	);

	//vr::VREvent_t vrEvent;
	//vr::VROverlay()->PollNextOverlayEvent(m_ulOverlayHandle, &vrEvent, sizeof(vrEvent));

	//_RPTF1(_CRT_WARN, "VREvent %d\n", vrEvent.eventType);

}

void OpenVRHelper::Init(ID3D11Texture2D* pTex)
{
	// COpenVROverlayController::Init
	vr::EVRInitError eError = vr::VRInitError_None;
	m_pHMD = vr::VR_Init( &eError, vr::VRApplication_Overlay );
	if (eError == vr::VRInitError_None)
	{
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
				//overlayError = vr::VROverlay()->ShowOverlay(m_ulOverlayHandle);
				if (overlayError == vr::VROverlayError_None)
				{
					overlayError = vr::VROverlay()->SetOverlayWidthInMeters( m_ulOverlayHandle, 1.5f );
					overlayError = vr::VROverlay()->SetOverlayFlag(m_ulOverlayHandle, vr::VROverlayFlags_VisibleInDashboard, true);
					if (overlayError == vr::VROverlayError_None)
					{
						overlayError = vr::VROverlay()->SetOverlayInputMethod(m_ulOverlayHandle, vr::VROverlayInputMethod_Mouse);
						if (overlayError == vr::VROverlayError_None)
						{
							overlayError = SetOverlayTexture(pTex);

							//if (overlayError == vr::VROverlayError_None)
							{
								char rgchKey[vr::k_unVROverlayMaxKeyLength] = { 0 };
								vr::VROverlay()->GetOverlayKey(m_ulOverlayHandle, rgchKey, ARRAYSIZE(rgchKey), &overlayError);
								//if (overlayError == vr::VROverlayError_None)
								{
									vr::VROverlay()->ShowDashboard(rgchKey);
								}
							}
						}
					}
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
		vr::ColorSpace_Auto
	};

	return vr::VROverlay()->SetOverlayTexture(m_ulOverlayHandle, &overlayTextureDX11);
}