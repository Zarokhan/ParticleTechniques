#include "Camera.h"
#define DEG_RAD_CONSTANT 0.0174532925f

Camera::Camera()
{
	m_position = XMFLOAT3(0, 0, 0);
	m_rotation = XMFLOAT3(0, 0, 0);
	m_speed = 15.f;
}

Camera::Camera(const Camera& other)
{
}

Camera::~Camera()
{
}

void Camera::SetSpeed(const float speed)
{
	this->m_speed = speed;
}

float Camera::GetSpeed()
{
	return this->m_speed;
}

void Camera::Render()
{
	XMVECTOR position, YVector, ZVector;
	XMMATRIX rotationMatrix;

	position = XMLoadFloat3(&m_position);
	YVector = XMLoadFloat3(&XMFLOAT3(0.f, 1.f, 0.f));
	ZVector = XMLoadFloat3(&XMFLOAT3(0.f, 0.f, 1.f));

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	rotationMatrix = XMMatrixRotationRollPitchYaw(m_rotation.x * DEG_RAD_CONSTANT, m_rotation.y * DEG_RAD_CONSTANT, m_rotation.z * DEG_RAD_CONSTANT);

	ZVector = XMVector3TransformCoord(ZVector, rotationMatrix);
	YVector = XMVector3TransformCoord(YVector, rotationMatrix);
	ZVector = XMVectorAdd(position, ZVector);

	m_viewMatrix = XMMatrixLookAtLH(position, ZVector, YVector);
}

void Camera::RenderReflection(float height)
{
	XMFLOAT3 up, position, lookAt;
	float radians;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	position.x = m_position.x;
	position.y = -m_position.y + (height * 2.0f);
	position.z = m_position.z;

	radians = m_rotation.y * 0.0174532925f;

	lookAt.x = sinf(radians) + m_position.x;
	lookAt.y = position.y;
	lookAt.z = cosf(radians) + m_position.z;

	m_reflectionViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&position), XMLoadFloat3(&lookAt), XMLoadFloat3(&up));
}
