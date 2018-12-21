#pragma once

#include "openvr/headers/openvr.h"

struct ID3D11Texture2D;
struct ID3D11DeviceContext;


class OpenVRHelper
{
public:
	OpenVRHelper() :
		m_pHMD(nullptr)
	{
	}
	~OpenVRHelper()
	{
	}

	void Init(ID3D11Texture2D* pTex);
	vr::VROverlayError SetOverlayTexture(ID3D11Texture2D* pTex);

private:
	vr::IVRSystem * m_pHMD;
	vr::VROverlayHandle_t m_ulOverlayHandle;
	vr::VROverlayHandle_t m_ulOverlayThumbnailHandle;
};

