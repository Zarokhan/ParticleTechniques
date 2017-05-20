#ifndef _RENDERTEXTURE_H_
#define _RENDERTEXTURE_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>
#include "../DirectX.h"
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: RenderTexture
////////////////////////////////////////////////////////////////////////////////
class RenderTexture
{
public:
	RenderTexture();
	RenderTexture(const RenderTexture&);
	~RenderTexture();

	bool Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear);

	void SetRenderTarget(ID3D11DeviceContext* deviceContext) { deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView); deviceContext->RSSetViewports(1, &m_viewport); }
	void ClearRenderTarget(ID3D11DeviceContext* deviceContext, float red, float green, float blue, float alpha);
	ID3D11ShaderResourceView* GetShaderResourceView() { return m_shaderResourceView; }

	XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }
	XMMATRIX GetOrthoMatrix() { return m_orthoMatrix; }

private:
	ID3D11Texture2D* m_renderTargetTexture;
	ID3D11RenderTargetView* m_renderTargetView;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;

	D3D11_VIEWPORT m_viewport;
	XMMATRIX m_projectionMatrix;
	XMMATRIX m_orthoMatrix;
};

#endif