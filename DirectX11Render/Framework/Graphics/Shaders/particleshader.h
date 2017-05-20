#ifndef _PARTICLESHADER_H_
#define _PARTICLESHADER_H_

#include <d3dcompiler.h>
#include <directxmath.h>
// Maybe need d3dx11async.h
#include <fstream>
#include "../../DirectX.h"
#include "../Direct3DManager.h"
#include "../ParticleSystem/ParticleSystem.h"
#include "../Camera.h"

using namespace DirectX;

class ParticleShader
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

public:
	ParticleShader();
	ParticleShader(const ParticleShader&);
	~ParticleShader();

	bool Initialize(ID3D11Device3* device, HWND hwnd);
	bool Render(Direct3DManager* direct, ParticleSystem* particlesystem, Camera* camera);

private:
	bool InitializeShader(ID3D11Device3* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename, WCHAR * gsFilename);
	void OutputShaderErrorMsg(ID3D10Blob* errorMsg, HWND hwnd, WCHAR* shaderFilename);

	bool SetShaderParameters(ID3D11DeviceContext* deviceContext, Camera * cameraPos, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture);
	void RenderShader(ID3D11DeviceContext* deviceContext, int vertexCount, int instanceCount, int indexCount);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11GeometryShader* m_geometryShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11SamplerState* m_sampleState;
};

#endif