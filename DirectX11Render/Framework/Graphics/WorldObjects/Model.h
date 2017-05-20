#ifndef _MODEL_H_
#define _MODEL_H_

//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <directxmath.h>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "../Texture.h"
#include "../../DirectX.h"

using namespace DirectX;

struct VertexType;
struct ModelType;
struct TempVertexType;
struct VectorType;

////////////////////////////////////////////////////////////////////////////////
// Super Class name: Model
////////////////////////////////////////////////////////////////////////////////
class Model
{
public:
	Model();
	Model(const Model&);
	virtual ~Model();

	void Render(ID3D11DeviceContext*);
	virtual void Update(const float& dt) {}

	ID3D11ShaderResourceView* GetTexture() const { return m_Texture->GetTexture(); }
	ID3D11ShaderResourceView* GetNormalTexture() const { return m_NormalTexture->GetTexture(); }
	XMMATRIX GetWorldMatrix() const { return m_worldMatrix; }
	inline int GetIndexCount() const { return m_indexCount; }
	inline float GetTextureScale() const { return m_textureScale; }

	inline void SetScale(XMFLOAT3& scale) { m_scale = XMFLOAT3(scale); }
	inline void SetPosition(XMFLOAT3& position) { m_position = XMFLOAT3(position); }
	inline void SetRotation(XMFLOAT3& rotation) { m_rotation = XMFLOAT3(rotation); }
	inline void SetTextureScale(float scale) { m_textureScale = scale; }

	inline XMFLOAT3 GetPosition() const { return m_position; } const
	inline XMFLOAT3 GetRotation() const { return m_rotation; } const
	inline XMFLOAT3 GetScale() const { return m_scale; } const

	inline void TranslateRotation(const XMFLOAT3& rotation)
	{
		XMVECTOR vec1 = XMLoadFloat3(&m_rotation);
		XMVECTOR vec2 = XMLoadFloat3(&rotation);
		XMVECTOR result = vec1 + vec2;
		XMStoreFloat3(&m_rotation, result);
	}

	inline void TranslatePosition(XMFLOAT3& vec)
	{
		XMVECTOR vec1 = XMLoadFloat3(&m_position);
		XMVECTOR vec2 = XMLoadFloat3(&vec);
		XMVECTOR result = vec1 + vec2;
		XMStoreFloat3(&m_position, result);
	}

private:
	void RenderBuffers(ID3D11DeviceContext*);

protected:
	bool InitializeBuffers(ID3D11Device*);
	bool LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* texPath, const wchar_t* normalPath);

	// Used for calculating normal, tangent and binormal for bumpmap(normalmap)
	void CalculateModelVectors();
private:
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);
	void CalculateNormal(VectorType, VectorType, VectorType&);

protected:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	Texture *m_Texture;
	Texture *m_NormalTexture;
	ModelType* m_model;

	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMFLOAT3 m_scale;
	XMMATRIX m_worldMatrix;
	float m_textureScale;
};

struct VertexType
{
	XMFLOAT3 position;
	XMFLOAT2 texture;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 binormal;
};

struct ModelType
{
	float x, y, z;
	float tu, tv;
	float nx, ny, nz;
	float tx, ty, tz;
	float bx, by, bz;
};

struct TempVertexType
{
	float x, y, z;
	float tu, tv;
	float nx, ny, nz;
};

struct VectorType
{
	float x, y, z;
};

#endif