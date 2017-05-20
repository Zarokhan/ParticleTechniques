#include "ParticleSys.h"


GPUP::ParticleSys::ParticleSys()
{
	m_texture = 0;
	m_noise = 0;
	m_streamOutVB = 0;
	m_drawVB = 0;
	m_initVB = 0;
	m_fx = 0;
	m_particleLayout = 0;
}

GPUP::ParticleSys::ParticleSys(const ParticleSys &other)
{
}

GPUP::ParticleSys::~ParticleSys()
{
	SAFE_RELEASE(m_streamOutVB);
	SAFE_RELEASE(m_drawVB);
	SAFE_RELEASE(m_initVB);
	SAFE_RELEASE(m_particleLayout);
	SAFE_DELETE(m_fx);
	SAFE_DELETE(m_texture);
	SAFE_DELETE(m_noise);
}

bool GPUP::ParticleSys::Initialize(Direct3DManager* direct3D, HWND hwnd, WCHAR *particleFileName, WCHAR *noiseFileName)
{
	bool result;
	result = LoadTexture(direct3D->GetDevice(), direct3D->GetDeviceContext(), particleFileName, noiseFileName);
	ASSERT(result);

	result = InitializeParticleSystem();
	ASSERT(result);

	result = InitializeBuffers(direct3D->GetDevice(), direct3D->GetDeviceContext());
	ASSERT(result);

	m_fx = new ParticleEffect();

	result = m_fx->Initialize(direct3D->GetDevice(), hwnd, L"Shaders/snow.fx");
	ASSERT(result);

	D3DX11_PASS_DESC passDesc;

	D3D11_INPUT_ELEMENT_DESC ParticleLayout[6] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "OFFSET", 0, DXGI_FORMAT_R32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TYPE", 0, DXGI_FORMAT_R32_UINT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	FAIL(m_fx->m_streamOutTech->GetPassByIndex(0)->GetDesc(&passDesc));
	FAIL(direct3D->GetDevice()->CreateInputLayout(ParticleLayout, 6, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &m_particleLayout));

	return true;
}



bool GPUP::ParticleSys::LoadTexture(ID3D11Device3 * device, ID3D11DeviceContext* deviceContext, WCHAR *particleFileName, WCHAR *noiseFileName)
{
	bool result;

	m_texture = new Texture();
	if (!m_texture)
		return false;

	result = m_texture->Initialize(device, deviceContext, particleFileName);
	if (!result)
		return false;
	
	m_noise = new Texture();
	if (!m_noise)
		return false;

	result = m_noise->Initialize(device, deviceContext, noiseFileName);
	if (!result)
		return false;



	return true;
}

bool GPUP::ParticleSys::InitializeParticleSystem()
{
	m_maxParticles = 1000000;
	m_particlespersecond = 2000;
	m_particleMaxAge = 30.0f;
	m_firstRun = true;
	m_age = 0.0f;
	m_emitterAge = 0.0f;
	m_timeStep = 0.0f;

	m_eyePos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_emitPos = XMFLOAT3(-50.0f, 30.0f, -50.0f); // Good spawn
	m_emitDir = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_particleSize = XMFLOAT2(0.1f, 0.1f);

	return true;
}

bool GPUP::ParticleSys::InitializeBuffers(ID3D11Device3 *device, ID3D11DeviceContext* deviceContext)
{
	VertexType* vertices;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	HRESULT result;

	m_vertexCount = 1;	// One vertex for emitter

	vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	memset(vertices, 0, sizeof(VertexType) * m_vertexCount);

	unsigned long long fl = sizeof(VertexType);

	// Emitter vertex
	ZeroMemory(&vertices[0], sizeof(VertexType));
	vertices[0].age = 0.0f;
	vertices[0].type = 0;
	vertices[0].offset = 0;
	vertices[0].size = m_particleSize;
	vertices[0].velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	vertices[0].position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	// Init VB
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * 1;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_initVB);
	FAIL(result);

	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_maxParticles;

	FAIL(device->CreateBuffer(&vertexBufferDesc, 0, &m_drawVB));
	FAIL(device->CreateBuffer(&vertexBufferDesc, 0, &m_streamOutVB));

	SAFE_DELETE_ARRAY(vertices);

	return true;
}

void GPUP::ParticleSys::Reset()
{
	m_firstRun = true;
	m_age = 0.0f;
}

void GPUP::ParticleSys::Frame(const float dt)
{
	m_timeStep = dt;
	m_age += dt;
	// Check emulated particles
	m_emitterAge += dt;
	//for (int i = 0; i < m_currentparticles.size(); i++)
	//{
	//	m_currentparticles[i] += dt;
	//	if (m_currentparticles[i] > m_particleMaxAge)
	//	{
	//		std::swap(m_currentparticles.begin() + i, m_currentparticles.begin() + m_currentparticles.size() - 1);
	//		m_currentparticles.erase(m_currentparticles.begin() + m_currentparticles.size() - 1);
	//		//m_currentparticles.erase(m_currentparticles.begin() + i);
	//	}
	//}
}

bool GPUP::ParticleSys::Render(Direct3DManager* direct3d, Camera* camera)
{
	HRESULT hr;
	ID3D11DeviceContext* deviceContext = direct3d->GetDeviceContext();
	XMMATRIX VP;
	//VP = camera->GetViewMatrix();
	VP = XMMatrixMultiply(camera->GetViewMatrix(), direct3d->GetProjectionMatrix()); // Maybe need to add projection matrix
	//VP = XMMatrixTranspose(VP);

	m_fx->SetViewProj(VP);
	m_fx->SetGameTime(m_age);
	m_fx->SetTimeStep(m_timeStep);
	m_fx->SetEyePosW(camera->GetPosition());
	m_fx->SetEmitPosW(m_emitPos);
	m_fx->SetEmitDirW(m_emitDir);
	m_fx->SetTexArray(m_texture->GetTexture());
	m_fx->SetRandomTex(m_noise->GetTexture());
	m_fx->SetParticlePerSecond(m_particlespersecond);
	m_fx->SetParticleAge(m_particleMaxAge);

	// Calc particles
	int i = 0;
	while (m_emitterAge > 1.0f / ((float)m_particlespersecond))
	{
		if (i < 99)
			m_currentparticles.push_back(0.0f);
		else
			break;

		m_emitterAge -= 1.0f / ((float)m_particlespersecond);
		i++;
	}
	
	deviceContext->IASetInputLayout(m_particleLayout);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	
	UINT stride = sizeof(VertexType);
	UINT offset = 0;

	if (m_firstRun)
		deviceContext->IASetVertexBuffers(0, 1, &m_initVB, &stride, &offset);
	else
		deviceContext->IASetVertexBuffers(0, 1, &m_drawVB, &stride, &offset);

	deviceContext->SOSetTargets(1, &m_streamOutVB, &offset);
	 
	D3DX11_TECHNIQUE_DESC techDesc;
	m_fx->m_streamOutTech->GetDesc(&techDesc);
	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		hr = m_fx->m_streamOutTech->GetPassByIndex(i)->Apply(0, deviceContext);
		FAIL(hr);

		if (m_firstRun)
		{
			deviceContext->Draw(1, 0);
			m_firstRun = false;
		}
		else
		{
			deviceContext->DrawAuto();
		}
	}

	ID3D11Buffer* bufferArray[1] = { 0 };
	deviceContext->SOSetTargets(1, bufferArray, &offset);

	std::swap(m_drawVB, m_streamOutVB);

	deviceContext->IASetVertexBuffers(0, 1, &m_drawVB, &stride, &offset);

	m_fx->m_drawTech->GetDesc(&techDesc);

	for (UINT i = 0; i < techDesc.Passes; ++i)
	{
		m_fx->m_drawTech->GetPassByIndex(i)->Apply(0, deviceContext);

		deviceContext->DrawAuto();
	}
	return true;
}

void GPUP::ParticleSys::SetEyePos(const XMFLOAT3& eyePosW)
{
	m_eyePos = eyePosW;
}

void GPUP::ParticleSys::SetEmitPos(const XMFLOAT3& emitPosW)
{
	m_emitPos = emitPosW;
}

void GPUP::ParticleSys::SetEmitDir(const XMFLOAT3& emitDirW)
{
	m_emitDir = emitDirW;
}

// --------------------------------
//
//	PARTICLE EFFECT
//
// --------------------------------

GPUP::ParticleEffect::ParticleEffect()
{
}

GPUP::ParticleEffect::~ParticleEffect()
{
	SAFE_RELEASE(m_drawTech);
	SAFE_RELEASE(m_viewProj);
	SAFE_RELEASE(m_gameTime);
	SAFE_RELEASE(m_timeStep);
	SAFE_RELEASE(m_eyePosW);
	SAFE_RELEASE(m_emitPosW);
	SAFE_RELEASE(m_emitDirW);
	SAFE_RELEASE(m_texArray);
	SAFE_RELEASE(m_randomTex);
	SAFE_RELEASE(m_streamOutTech);
	SAFE_RELEASE(m_particlespersec);
	SAFE_RELEASE(m_fx); // Root. Needs to be released last
}

bool GPUP::ParticleEffect::Initialize(ID3D11Device* device, HWND hwnd, WCHAR* fileName)
{
	ID3DBlob* errorMsg = 0; 
	HRESULT hr;
	hr = D3DX11CompileEffectFromFile(fileName, NULL, NULL, D3D10_SHADER_ENABLE_STRICTNESS, NULL, device, &m_fx, &errorMsg);
	if (FAILED(hr))
	{
		// If the shader failed to compile it should have writen something to the error message.
		if (errorMsg)
		{
			OutputShaderErrorMessage(errorMsg, hwnd, fileName);
		}
		// If there was  nothing in the error message then it simply could not find the shader file itself.
		else
		{
			MessageBox(hwnd, fileName, L"Missing FX-Shader File", MB_OK);
		}

		return false;
	}

	SAFE_RELEASE(errorMsg);

	m_streamOutTech = m_fx->GetTechniqueByName("StreamOutTech");
	m_drawTech = m_fx->GetTechniqueByName("DrawTech");
				 
	m_viewProj = m_fx->GetVariableByName("gViewProj")->AsMatrix();
	m_gameTime = m_fx->GetVariableByName("gGameTime")->AsScalar();
	m_timeStep = m_fx->GetVariableByName("gTimeStep")->AsScalar();
	m_particlespersec = m_fx->GetVariableByName("gParticlesPerSecond")->AsScalar();
	m_particleage = m_fx->GetVariableByName("gParticleAge")->AsScalar();
	m_eyePosW = m_fx->GetVariableByName("gEyePosW")->AsVector();
	m_emitPosW = m_fx->GetVariableByName("gEmitPosW")->AsVector();
	m_emitDirW = m_fx->GetVariableByName("gEmitDirW")->AsVector();
	m_texArray = m_fx->GetVariableByName("gTexArray")->AsShaderResource();
	m_randomTex = m_fx->GetVariableByName("gRandomTex")->AsShaderResource();

	return true;
}

void GPUP::ParticleEffect::OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	std::ofstream fout;

	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}

	// Close the file.
	fout.close();

	// Release the error message.
	SAFE_RELEASE(errorMessage);

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}