#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <d3d11.h>
#include <stdio.h>
#include "TextureLoader\WICTextureLoader.h"
#include "TextureLoader\DDSTextureLoader.h"
#include "../DirectX.h"

class Texture
{
private:
	ID3D11Resource* m_Texture;
	ID3D11ShaderResourceView* m_TextureView;

public:
	Texture()
	{
		m_Texture = 0;
		m_TextureView = 0;
	}

	~Texture()
	{
		SAFE_RELEASE(m_Texture);
		SAFE_RELEASE(m_TextureView);
	}

	inline bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* path)
	{
		HRESULT hr = DirectX::CreateWICTextureFromFile(device, deviceContext, path, &m_Texture, &m_TextureView);
		return SUCCEEDED(hr);
	}

	inline bool InitializeDDS(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* path)
	{
		HRESULT hr = DirectX::CreateDDSTextureFromFile(device, deviceContext, path, &m_Texture, &m_TextureView);
		return SUCCEEDED(hr);
	}

public:
	inline ID3D11ShaderResourceView* GetTexture() const { return m_TextureView; }
	ID3D11Resource* GetResource() const { return m_Texture; }
};

#endif