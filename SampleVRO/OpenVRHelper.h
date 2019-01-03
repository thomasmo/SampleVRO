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
		m_ulOverlayThumbnailHandle(vr::k_ulOverlayHandleInvalid)
	{
	}
	~OpenVRHelper()
	{
	}

	void Init(int32_t *pnAdapterIndex );
	void CreateOverlay(ID3D11Texture2D* pTex);
	vr::VROverlayError SetOverlayTexture(ID3D11Texture2D* pTex);

private:
	vr::IVRSystem * m_pHMD;
	int32_t m_dxgiAdapterIndex;
	vr::VROverlayHandle_t m_ulOverlayHandle;
	vr::VROverlayHandle_t m_ulOverlayThumbnailHandle;
};

