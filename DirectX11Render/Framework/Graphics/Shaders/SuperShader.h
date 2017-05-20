#ifndef _SUPERSHADER_H_
#define _SUPERSHADER_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <fstream>
#include "../Light.h"
#include "../../DirectX.h"
#include "../Direct3DManager.h"
#include "../WorldObjects\Model.h"
#include "../Camera.h"
using namespace DirectX;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Class name: SuperShader
////////////////////////////////////////////////////////////////////////////////
class SuperShader
{
private:
	// cbuffer needs to match the vertex shader, needs to match the typedefs
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
		XMMATRIX lightView;
		XMMATRIX lightProjection;
	};

	struct ReflectionBufferType
	{
		XMMATRIX reflectionMatrix;
	};

	struct ObjectBufferType
	{
		float textureScale;
		XMFLOAT3 padding;
	};

	struct CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 ambientColor;
		XMFLOAT4 diffuseColor;
		XMFLOAT3 lightDirection;
		float specularPower;
		XMFLOAT4 specularColor;
	};

	struct LightBufferVSType
	{
		XMFLOAT3 lightPosition;
		float padding;
	};

public:
	SuperShader();
	SuperShader(const SuperShader&);
	~SuperShader();

	bool Initialize(ID3D11Device*, HWND);
	bool Render(Direct3DManager& direct3D, Camera& camera, const Light& light, Model& model, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix, ID3D11ShaderResourceView* depthMapTexture, XMFLOAT3 lightPosition, XMMATRIX reflectionMatrix, ID3D11ShaderResourceView* reflectionTexture = nullptr);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR*);
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, WCHAR*);

	bool SetShaderParameters(Direct3DManager& direct3D, Camera& camera, const Light& light, Model& model, XMMATRIX lightViewMatrix, XMMATRIX lightProjectionMatrix, ID3D11ShaderResourceView* depthMapTexture, XMFLOAT3 lightPosition, XMMATRIX reflectionMatrix, ID3D11ShaderResourceView* reflectionTexture);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11Buffer* m_cameraBuffer;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_objectBuffer;
	ID3D11Buffer* m_reflectionBuffer;
	ID3D11Buffer* m_lightVSBuffer;
};

#endif