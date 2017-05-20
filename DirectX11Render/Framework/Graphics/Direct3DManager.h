////////////////////////////////////////////////////////////////////////////////
// Filename: d3dclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _DIRECT3DMANAGER_H_
#define _DIRECT3DMANAGER_H_

/////////////
// LINKING //
/////////////
#pragma comment(lib, "d3d11.lib")
//#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

//////////////
// INCLUDES //
//////////////
#include <d3d11_3.h>
#include <directxmath.h>
#include "../DirectX.h"
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: Direct3DManager
////////////////////////////////////////////////////////////////////////////////
class Direct3DManager
{
public:
	Direct3DManager();
	Direct3DManager(const Direct3DManager&);
	~Direct3DManager();

	bool Initialize(int, int, bool, HWND, bool, float, float);

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device3* GetDevice() { return m_device3; }
	ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext; }

	ID3D11RenderTargetView* GetRenderTarget() { return m_renderTargetView; }

	XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }
	XMMATRIX GetWorldMatrix() { return m_worldMatrix; }
	XMMATRIX GetOrthoMatrix() { return m_orthoMatrix; }

	void GetVideoCardInfo(char*, int&);

	void TurnZBufferOn() { m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1); }
	void TurnZBufferOff() { m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1); }

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

	ID3D11DepthStencilView* GetDepthStencilView() { return m_depthStencilView; }
	void SetBackBufferRenderTarget() { m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView); }

	void ResetViewport() { m_deviceContext->RSSetViewports(1, &m_viewport); }

private:
	bool m_vsync_enabled;
	int m_videoCardMemory;
	char m_videoCardDescription[128];
	IDXGISwapChain* m_swapChain;
	ID3D11Device* m_device;
	ID3D11Device3* m_device3;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilView* m_depthStencilView;
	ID3D11RasterizerState2* m_rasterState;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_worldMatrix;
	XMMATRIX m_orthoMatrix;
	D3D11_VIEWPORT m_viewport;

	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11BlendState* m_alphaDisableBlendingState;
	ID3D11BlendState* m_alphaEnableBlendingState;
};

#endif