#ifndef _PARTICLESYSTEM_H_
#define _PARTICLESYSTEM_H_

#include "../Direct3DManager.h"
#include <directxmath.h>
#include "../Texture.h"
#include <algorithm>
#include <thread>
#include <vector>

using namespace DirectX;

class ParticleSystem
{
private:
	struct ParticleType
	{
		XMFLOAT3 initialPos;
		XMFLOAT3 initialVel;
		float age;
		float offset;
	};

	struct InstanceType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

public:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT4 color;
	};

public:
	ParticleSystem();
	ParticleSystem(const ParticleSystem&);
	~ParticleSystem();

	bool Initialize(Direct3DManager*, WCHAR*, int);
	bool Frame(float, ID3D11DeviceContext*);
	void Render(ID3D11DeviceContext*);

	//ID3D11UnorderedAccessView* GetUAV() { return m_uav; }
	ID3D11ShaderResourceView* GetTexture() { return m_texture->GetTexture(); }
	int GetIndexCount() { return m_indexCount; }
	int GetVertexCount(){ return m_vertexCount; }
	int GetInstaceCount(){ return m_instanceCount; }
	int GetCurrentParticleCount() { return m_currentParticleCount; }
	int GetParticlesPerSecond() { return m_particlePerSecond; }
	void SetParticlesPerSecond(int value) { m_particlePerSecond = value; }
private:
	// Initialize
	bool LoadTexture(ID3D11Device3*, ID3D11DeviceContext*, WCHAR*);
	bool InitializeParticleSystem(int);
	bool InitializeBuffers(ID3D11Device3*, ID3D11DeviceContext*);
	void CalculateInstancePositions(int, int);

	// Particle methods
	void EmitParticles(float);
	void UpdateParticles(float);
	void SpawnParticle();

	// Render & Update
	bool UpdateBuffers(ID3D11DeviceContext*);
	void RenderBuffers(ID3D11DeviceContext*);
private:

	float m_particleDeviationX, m_particleDeviationY, m_particleDeviationZ;
	float m_particleVeclocity, m_particleVelocityVariation;
	float m_particleSize, m_particlePerSecond;
	int m_maxParticles;

	int m_currentParticleCount;
	float m_accumulatedTime;

	ID3D11UnorderedAccessView* m_uav;
	ID3D11Texture2D* m_uavTex;

	Texture* m_texture;
	ParticleType* m_particleList;

	int m_vertexCount, m_indexCount, m_instanceCount;
	InstanceType *m_instance;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer, *m_instanceBuffer;

	std::vector<std::thread> m_threads;
};

#endif