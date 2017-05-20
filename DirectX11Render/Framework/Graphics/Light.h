#ifndef _LIGHT_H_
#define _LIGHT_H_

#define D3DX_PI 3.14159265359

//////////////
// INCLUDES //
//////////////
#include <DirectXMath.h>

using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: Light
////////////////////////////////////////////////////////////////////////////////
class Light
{
	Light(const Light&);
public:
	Light() {}
	~Light() {}

	inline void SetAmbientColor(float red, float green, float blue, float alpha) { m_ambientColor = XMFLOAT4(red, green, blue, alpha); }
	inline void SetDiffuseColor(float red, float green, float blue, float alpha) { m_diffuseColor = XMFLOAT4(red, green, blue, alpha); }
	inline void SetSpecularColor(float red, float green, float blue, float alpha) { m_specularColor = XMFLOAT4(red, green, blue, alpha); }
	inline void SetDirection(float x, float y, float z) { m_direction = XMFLOAT3(x, y, z); }
	inline void SetSpecularPower(float power) { m_specularPower = power; }

	inline void SetPosition(float x, float y, float z) { m_position = XMFLOAT3(x, y, z); }
	inline void SetLookAt(float x, float y, float z)
	{ 
		m_lookAt = XMFLOAT3(x, y, z);
		return;
		XMVECTOR vec1 = XMLoadFloat3(&m_lookAt);
		XMVECTOR vec2 = XMLoadFloat3(&m_position);
		XMVECTOR result = vec1 - vec2;
		XMVector3Normalize(result);
		XMStoreFloat3(&m_direction, result);
	}

	inline XMFLOAT4 GetAmbientColor() const { return m_ambientColor; }
	inline XMFLOAT4 GetDiffuseColor() const { return m_diffuseColor; }
	inline XMFLOAT4 GetSpecularColor() const { return m_specularColor; }
	inline XMFLOAT3 GetDirection() const { return m_direction; }
	inline float GetSpecularPower() const { return m_specularPower; }
	inline XMFLOAT3 GetPosition() const { return m_position; }

	inline XMMATRIX GetViewMatrix() { return m_viewMatrix; }
	inline XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; }

	void TranslatePosition(XMFLOAT3 vec)
	{
		XMVECTOR vec1 = XMLoadFloat3(&m_position);
		XMVECTOR vec2 = XMLoadFloat3(&vec);
		XMVECTOR result = vec1 + vec2;
		XMStoreFloat3(&m_position, result);
	}

	void GenerateViewMatrix()
	{
		XMFLOAT3 up;

		up.x = 0.0f;
		up.y = 1.0f;
		up.z = 0.0f;

		m_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&m_position), XMLoadFloat3(&m_lookAt), XMLoadFloat3(&up));
	}

	void GenerateProjectionMatrix(float screenDepth, float screenNear)
	{
		float fieldOfView, screenAspect;

		fieldOfView = (float)D3DX_PI / 4.0f;
		screenAspect = 1.0f;

		// Create the projection matrix for the light.
		m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
	}

private:
	XMFLOAT4 m_ambientColor;
	XMFLOAT4 m_diffuseColor;
	XMFLOAT3 m_direction;
	XMFLOAT4 m_specularColor;
	float m_specularPower;

	XMFLOAT3 m_position;
	XMFLOAT3 m_lookAt;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_projectionMatrix;
};

#endif