#ifndef _CAMERA_H_
#define _CAMERA_H_

//////////////
// INCLUDES //
//////////////
#include <directxmath.h>
using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: Camera
////////////////////////////////////////////////////////////////////////////////
class Camera
{
public:
	Camera();
	Camera(const Camera&);
	~Camera();

	inline void MoveForward(const float& dt, const float& val = 1)
	{
		XMVECTOR temp = XMLoadFloat3(&m_position) + XMMatrixInverse(nullptr, m_viewMatrix).r[2] * XMLoadFloat3(&XMFLOAT3(m_speed, m_speed, m_speed)) * dt * val;
		XMStoreFloat3(&m_position, temp);
	}

	inline void Strafe(const float& dt, const float& val = 1)
	{
		XMVECTOR temp = XMLoadFloat3(&m_position) + XMMatrixInverse(nullptr, m_viewMatrix).r[0] * XMLoadFloat3(&XMFLOAT3(m_speed, m_speed, m_speed)) * dt * val;
		XMStoreFloat3(&m_position, temp);
	}

	inline void AddRotation(const float deltaX, const float deltaY)
	{
		m_rotation.x += deltaY;
		m_rotation.y += deltaX;
	}

	inline void SetPosition(float x, float y, float z) { m_position = XMFLOAT3(x, y, z); }
	inline void SetRotation(float x, float y, float z) { m_rotation = XMFLOAT3(x, y, z); }
	inline XMFLOAT3 GetPosition() { return m_position; }
	inline XMFLOAT3 GetRotation() { return m_rotation; }
	inline XMMATRIX GetViewMatrix() { return m_viewMatrix; }
	inline XMMATRIX GetReflectionViewMatrix() { return m_reflectionViewMatrix; }
	void SetSpeed(const float speed);
	float GetSpeed();

	void Render();
	void RenderReflection(float);

private:
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMMATRIX m_viewMatrix;
	XMMATRIX m_reflectionViewMatrix;

	float m_speed;
};

#endif