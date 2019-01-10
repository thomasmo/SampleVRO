// DrawHelper.cpp
//
// This file contains code to draw to the screen (via a swapchain in D3D), specifically a green ellipse as
// well as any text typed by the user.
//
// Code in this file is based upon the following samples
// - https://docs.microsoft.com/en-us/windows/desktop/Direct2D/how-to--draw-text
// - https://docs.microsoft.com/en-us/windows/desktop/direct2d/direct2d-and-direct3d-interoperation-overview

#include "stdafx.h"
#include "DrawHelper.h"

#include "OpenVRHelper.h"
#include <wincodec.h>
#include "dxtk/Inc/ScreenGrab.h"

DrawHelper::DrawHelper():
	pFactory(nullptr),
	pDevice2d(nullptr),
	pDevice2dContext(nullptr),
	pRenderTarget(nullptr),
	pRenderTargetBitmap(nullptr),
	pRenderTargetHwnd(nullptr),
	pBrush(nullptr),
	pBrushText(nullptr),
	pBrushClick(nullptr),
	pDevice3d(nullptr),
	pDevice3dContext(nullptr),
	pTex(nullptr),
	pDxgiDevice(nullptr),
	pSurface(nullptr),
	pSwapChain(nullptr),
	pDWriteFactory(nullptr),
	pTextFormat(nullptr)
{
}

DrawHelper::~DrawHelper()
{
}

// Per-instance
HRESULT DrawHelper::Setup()
{
	D2D1_FACTORY_OPTIONS options;
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
	return D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		options,
		&pFactory
	);
}

// Per-instance
void DrawHelper::Shutdown()
{
	DiscardGraphicsResources();
	SafeRelease(&pFactory);
	SafeRelease(&pDevice2d);
	SafeRelease(&pDevice2dContext);
	SafeRelease(&pDevice3d);
	SafeRelease(&pDevice3dContext);
	SafeRelease(&pDxgiDevice);
}

void DrawHelper::DiscardGraphicsResources()
{
	SafeRelease(&pRenderTarget);
	SafeRelease(&pRenderTargetBitmap);
	SafeRelease(&pRenderTargetHwnd);
	SafeRelease(&pBrush);
	SafeRelease(&pBrushText);
	SafeRelease(&pBrushClick);
	SafeRelease(&pTex);
	SafeRelease(&pSurface);
	SafeRelease(&pSwapChain);
	SafeRelease(&pDWriteFactory);
	SafeRelease(&pTextFormat);
}

HRESULT DrawHelper::CreateD3DResources(HWND hwndMain, HWND hwndOVR, OpenVRHelper* povrHelper)
{
	RECT rc;
	GetClientRect(hwndMain, &rc);

	// Ask OpenVR which adapter it uses so that the swapchain can be created
	// with the same adapter.
	int32_t dxgiAdapterIndex = -1;
	povrHelper->Init(hwndMain, hwndOVR, rc, &dxgiAdapterIndex);

	IDXGIFactory1 * pDxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&pDxgiFactory));
	if (hr == S_OK)
	{
		IDXGIAdapter1* pDxgiAdapter = nullptr;
		D3D_DRIVER_TYPE driverType = D3D_DRIVER_TYPE_HARDWARE;
		if (dxgiAdapterIndex >= 0)
		{
			hr = pDxgiFactory->EnumAdapters1(dxgiAdapterIndex, &pDxgiAdapter);
			if (hr == S_OK)
			{
				driverType = D3D_DRIVER_TYPE_UNKNOWN;
			}
		}

		if (hr == S_OK)
		{
			D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

			D3D_FEATURE_LEVEL levels [] = { D3D_FEATURE_LEVEL_11_1 };
			D3D_FEATURE_LEVEL deviceLevel = D3D_FEATURE_LEVEL_11_1;

			DXGI_SWAP_CHAIN_DESC descSwapChain = { 0 };
			descSwapChain.BufferDesc.Width = size.width;
			descSwapChain.BufferDesc.Height = size.height;
			descSwapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			descSwapChain.BufferDesc.RefreshRate.Numerator = 60;
			descSwapChain.BufferDesc.RefreshRate.Denominator = 1;
			descSwapChain.SampleDesc.Count = 1;
			descSwapChain.SampleDesc.Quality = 0;
			descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			descSwapChain.BufferCount = 1;
			descSwapChain.OutputWindow = hwndMain;
			descSwapChain.Windowed = TRUE;

			hr = D3D11CreateDeviceAndSwapChain(
				pDxgiAdapter,
				driverType,
				nullptr, // Software
				D3D11_CREATE_DEVICE_BGRA_SUPPORT,// | D3D11_CREATE_DEVICE_DEBUG,
				levels,
				ARRAYSIZE(levels),
				D3D11_SDK_VERSION,
				&descSwapChain,
				&pSwapChain,
				&pDevice3d,
				&deviceLevel,
				&pDevice3dContext
			);

			if (SUCCEEDED(hr))
			{
				// Get a DXGI device interface from the D3D device.
				hr = pDevice3d->QueryInterface(&pDxgiDevice);
				if (hr == S_OK)
				{
					hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pSurface));
					if (hr == S_OK)
					{
						hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pTex));
					}
				}
			}
			SafeRelease(&pDxgiAdapter);
		}

		SafeRelease(&pDxgiFactory);
	}

	return hr;
}

HRESULT DrawHelper::CreateD2DResources()
{
	// Create a D2D device from the DXGI device.
	HRESULT hr = pFactory->CreateDevice(
		pDxgiDevice,
		&pDevice2d
	);

	if (SUCCEEDED(hr))
	{
		// Create a device context from the D2D device.
		hr = pDevice2d->CreateDeviceContext(
			D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
			&pDevice2dContext
		);
	}
	
	return hr;
}

HRESULT DrawHelper::CreateDWriteResources()
{
	// Create a DirectWrite factory.
	HRESULT hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(pDWriteFactory),
		reinterpret_cast<IUnknown **>(&pDWriteFactory)
	);

	if (SUCCEEDED(hr))
	{
		static const WCHAR msc_fontName[] = L"Verdana";
		static const FLOAT msc_fontSize = 50;
		// Create a DirectWrite text format object.
		hr = pDWriteFactory->CreateTextFormat(
			msc_fontName,
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			msc_fontSize,
			L"", //locale
			&pTextFormat
		);
	}

	return hr;
}

// 
HRESULT DrawHelper::CreateGraphicsResources(HWND hwndMain, HWND hwndOVR, OpenVRHelper* povrHelper)
{
	HRESULT hr = S_OK;
	if (pRenderTarget == NULL)
	{
		hr = CreateD3DResources(hwndMain, hwndOVR, povrHelper);
		if (hr == S_OK)
		{
			hr = CreateDWriteResources();
			if (hr == S_OK)
			{
				D2D1_RENDER_TARGET_PROPERTIES props =
					D2D1::RenderTargetProperties(
						D2D1_RENDER_TARGET_TYPE_HARDWARE,
						D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
						96,
						96
					);

				hr = pFactory->CreateDxgiSurfaceRenderTarget(
					pSurface,
					&props,
					&pRenderTarget
				);

				if (hr == S_OK)
				{
					D2D1_BITMAP_PROPERTIES1 bitmapProps =
						D2D1::BitmapProperties1(
							D2D1_BITMAP_OPTIONS_TARGET,
							D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
							0.0f,
							0.0f,
							nullptr // colorContext
						);

					hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0.7f, 0), &pBrush);
					if (SUCCEEDED(hr))
					{
						hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.7f, 0, 0), &pBrushText);
						if (SUCCEEDED(hr))
						{
							hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 0, 0.7f), &pBrushClick);
							if (SUCCEEDED(hr))
							{
								CalculateLayout();
								povrHelper->CreateOverlay(pTex);
							}
						}
					}
				}
			}			
		}
	}

	return hr;
}

// Recalculate drawing layout when the size of the window changes.
void DrawHelper::CalculateLayout()
{
	if (pRenderTarget != NULL)
	{
		D2D1_SIZE_F size = pRenderTarget->GetSize();
		const float x = size.width / 2;
		const float y = size.height / 2;
		const float radius = min(x, y);
		ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);
	}
}

void DrawHelper::Draw(HWND hwndMain, HWND hwndOVR,  OpenVRHelper* povrHelper, WCHAR* pchTypeBuffer, UINT cchTypeBuffer, POINTS* pPoints, UINT cPoints)
{
	static float s_clickWidth = 5.0f;

	HRESULT hr = CreateGraphicsResources(hwndMain, hwndOVR, povrHelper);
	if (SUCCEEDED(hr))
	{
		pRenderTarget->BeginDraw();

		pRenderTarget->Clear( D2D1::ColorF(D2D1::ColorF::LightGray) );
		pRenderTarget->FillEllipse(ellipse, pBrush);

		D2D1_SIZE_F renderTargetSize = pRenderTarget->GetSize();
		pRenderTarget->DrawText(
			pchTypeBuffer,
			cchTypeBuffer,
			pTextFormat,
			D2D1::RectF(0, 0, renderTargetSize.width, renderTargetSize.height),
			pBrushText
		);

		if (cPoints == 1)
		{
			// For the first click, draw a dot at the click location
			pRenderTarget->FillEllipse(
				D2D1::Ellipse(D2D1::Point2F(pPoints[0].x, pPoints[0].y), s_clickWidth, s_clickWidth),
				pBrushClick
			);
		}
		else if (cPoints > 1)
		{
			// For subsequent clicks, draw lines connecting at each click location
			for (UINT n = 1; n < cPoints; n++)
			{
				pRenderTarget->DrawLine(
					D2D1::Point2F(pPoints[n - 1].x, pPoints[n - 1].y),
					D2D1::Point2F(pPoints[n].x, pPoints[n].y),
					pBrushClick,
					s_clickWidth
				);
			}
		}

		hr = pRenderTarget->EndDraw();
		if (hr == S_OK)
		{
			hr = pSwapChain->Present(0, 0);
			if (hr == S_OK)
			{
				if (hr == S_OK)
				{
					// TODO: Is this needed everytime the texture is updated? Maybe if swapchain contains multiple backings?
					povrHelper->SetOverlayTexture(pTex);
					// Save texture to disk (for debugging purposes)
					//Save(pDevice3dContext, pTex);
				}
			}
		}
		else
		{
			DiscardGraphicsResources();
		}
	}
}

// Save the texture to disk
void DrawHelper::Save(ID3D11DeviceContext* pContext, ID3D11Texture2D* pTex)
{
	// https://github.com/Microsoft/DirectXTK
	DirectX::SaveWICTextureToFile(
		pContext,
		pTex,
		GUID_ContainerFormatBmp,
		L"SampleVRO.bmp"
	);
}
