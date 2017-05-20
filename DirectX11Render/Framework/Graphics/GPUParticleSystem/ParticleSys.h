#ifndef _GPUPARTICLESYSTEM_H_
#define _GPUPARTICLESYSTEM_H_

#include <directxmath.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "../Direct3DManager.h"
#include "../Texture.h"
#include "d3dx11effect.h"
#include "../../DirectX.h"
#include "../Camera.h"

using namespace DirectX;

namespace GPUP
{
	class ParticleEffect
	{
	public:
		ParticleEffect();
		~ParticleEffect();

		bool Initialize(ID3D11Device* device, HWND hwnd, WCHAR* fileName);
		
		bool SetViewProj(CXMMATRIX M) { FAIL(m_viewProj->SetMatrix(reinterpret_cast<const float*>(&M))); }

		bool SetGameTime(float f) { FAIL(m_gameTime->SetFloat(f)); }
		bool SetTimeStep(float f) { FAIL(m_timeStep->SetFloat(f)); }

		bool SetEyePosW(const XMFLOAT3& v) { FAIL(m_eyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3))); }
		bool SetEmitPosW(const XMFLOAT3& v) { FAIL(m_emitPosW->SetRawValue(&v, 0, sizeof(XMFLOAT3))); }
		bool SetEmitDirW(const XMFLOAT3& v) { FAIL(m_emitDirW->SetRawValue(&v, 0, sizeof(XMFLOAT3))); }

		bool SetParticlePerSecond(int f) { FAIL(m_particlespersec->SetInt(f)); }
		bool SetParticleAge(float f) { FAIL(m_particleage->SetFloat(f)); }

		void SetTexArray(ID3D11ShaderResourceView* tex) { m_texArray->SetResource(tex); }
		void SetRandomTex(ID3D11ShaderResourceView* tex) { m_randomTex->SetResource(tex); }

	private:
		ID3DX11Effect* m_fx = 0;
		void OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hwnd, WCHAR* shaderFilename);
		

	public:
		ID3DX11EffectTechnique* m_streamOutTech = 0;
		ID3DX11EffectTechnique* m_drawTech = 0;

		ID3DX11EffectMatrixVariable* m_viewProj = 0;
		ID3DX11EffectScalarVariable* m_gameTime = 0;
		ID3DX11EffectScalarVariable* m_timeStep = 0;
		ID3DX11EffectScalarVariable* m_particlespersec = 0;
		ID3DX11EffectScalarVariable* m_particleage = 0;
		ID3DX11EffectVectorVariable* m_eyePosW = 0;
		ID3DX11EffectVectorVariable* m_emitPosW = 0;
		ID3DX11EffectVectorVariable* m_emitDirW = 0;
		ID3DX11EffectShaderResourceVariable* m_texArray = 0;
		ID3DX11EffectShaderResourceVariable* m_randomTex = 0;
	};

	class ParticleSys
	{
	private:
		struct VertexType
		{
			XMFLOAT3 position;
			XMFLOAT3 velocity;
			XMFLOAT2 size;
			float age;
			float offset;
			unsigned int type;
		};


	public:
		ParticleSys();
		~ParticleSys();

		bool Initialize(Direct3DManager*, HWND hwnd, WCHAR*, WCHAR*);
		void Frame(const float dt);
		bool Render(Direct3DManager* direct3d, Camera* camera);

		void Reset();
		void SetEyePos(const XMFLOAT3& eyePosW);
		void SetEmitPos(const XMFLOAT3& emitPosW);
		void SetEmitDir(const XMFLOAT3& emitDirW);

		ID3D11ShaderResourceView* GetTexture() { return m_texture->GetTexture(); }
		int GetVertexCount() const { return m_vertexCount; }
		float GetAge() const { return m_age; }
		int GetCurrentParticles() { return 0; }

		int GetParticlesPerSecond() { return m_particlespersecond; }
		void SetParticlesPerSecond(int value) { m_particlespersecond = value; }

	private:
		ParticleSys(const GPUP::ParticleSys&);
		// Initialize
		bool LoadTexture(ID3D11Device3*, ID3D11DeviceContext*, WCHAR*, WCHAR*);
		bool InitializeParticleSystem();
		bool InitializeBuffers(ID3D11Device3*, ID3D11DeviceContext*);
		
	
	private:
		int m_maxParticles;
		int m_vertexCount;
		int m_particlespersecond;
		float m_particleMaxAge;

		bool m_firstRun;

		float m_timeStep;
		float m_age;
		float m_emitterAge;

		std::vector<float> m_currentparticles;

		XMFLOAT3 m_eyePos;
		XMFLOAT3 m_emitPos;
		XMFLOAT3 m_emitDir;

		XMFLOAT2 m_particleSize;

		ParticleEffect* m_fx;
		
		ID3D11InputLayout* m_particleLayout;
		ID3D11Buffer* m_initVB;
		ID3D11Buffer* m_streamOutVB; // Stream out VB Buffer
		ID3D11Buffer* m_drawVB;
		Texture* m_texture;
		Texture* m_noise;
	};
}


#endif