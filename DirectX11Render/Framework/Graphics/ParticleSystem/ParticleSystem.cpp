#include "ParticleSystem.h"
#include <iostream>

ParticleSystem::ParticleSystem()
{
	m_texture = 0;
	m_particleList = 0;
	m_instance = 0;
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_instanceBuffer = 0;
	m_uav = 0;
	m_uavTex = 0;
}

ParticleSystem::ParticleSystem(const ParticleSystem &other)
{
}

ParticleSystem::~ParticleSystem()
{
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);

	SAFE_DELETE_ARRAY(m_particleList);
	SAFE_DELETE(m_texture);

	SAFE_RELEASE(m_instanceBuffer);
	SAFE_RELEASE(m_uav);
	SAFE_RELEASE(m_uavTex);
}

bool ParticleSystem::Initialize(Direct3DManager* direct3D, WCHAR *textureFileName, int maxParticles)
{
	bool results;
	results = LoadTexture(direct3D->GetDevice(), direct3D->GetDeviceContext(), textureFileName);
	if (!results)
		return false;

	results = InitializeParticleSystem(maxParticles);
	if (!results)
		return false;

	results = InitializeBuffers(direct3D->GetDevice(), direct3D->GetDeviceContext());
	if (!results)
		return false;

	// Set UAV 
	//ID3D11RenderTargetView* test = direct3D->GetRenderTarget();
	//direct3D->GetDeviceContext()->OMSetRenderTargetsAndUnorderedAccessViews(1, &test, direct3D->GetDepthStencilView(), 1, 1, &m_uav, NULL);

	return true;
}

bool ParticleSystem::Frame(float delta, ID3D11DeviceContext *deviceContext)
{
	bool results;

	EmitParticles(delta);

	UpdateParticles(delta);

	results = UpdateBuffers(deviceContext);
	if (!results)
		return false;
	return true;
}

void ParticleSystem::Render(ID3D11DeviceContext *deviceContext)
{
	RenderBuffers(deviceContext);
}

bool ParticleSystem::LoadTexture(ID3D11Device3 * device, ID3D11DeviceContext* deviceContext, WCHAR * filename)
{
	bool result;

	m_texture = new Texture();
	if (!m_texture)
		return false;

	result = m_texture->Initialize(device, deviceContext, filename);
	if (!result)
		return false;

	return true;
}

bool ParticleSystem::InitializeParticleSystem(int maxParticles)
{
	int i;

	m_particleDeviationX = 50.0f;
	m_particleDeviationY = 0.0f;
	m_particleDeviationZ = 50.0f;

	m_particleVeclocity = 1.0f;
	m_particleVelocityVariation = 0.0f;

	m_particleSize = 0.02f;

	m_particlePerSecond = 2000.f;

	m_maxParticles = m_instanceCount = maxParticles; // 5000 instances of one particle

	m_particleList = new ParticleType[m_maxParticles];
	if (!m_particleList)
		return false;

	m_currentParticleCount = 0;
	m_accumulatedTime = 0.f;

	return true;
}

bool ParticleSystem::InitializeBuffers(ID3D11Device3 *device, ID3D11DeviceContext* deviceContext)
{
	unsigned long* indices;
	VertexType* vertices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc, instanceBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData, instanceData;
	HRESULT result;

	m_vertexCount = 1;
	m_indexCount = m_vertexCount;

	vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

	memset(vertices, 0, sizeof(VertexType) * m_vertexCount);
	for (int i = 0; i < m_indexCount; ++i)
		indices[i] = i;

	int index = 0;
	vertices[index].position = XMFLOAT3(0, 0, 0);
	vertices[index].texture = XMFLOAT2(0, 0);
	vertices[index].color = XMFLOAT4(0, 0, 0, 0);
	++index;

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
		return false;

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
		return false;

	SAFE_DELETE_ARRAY(indices);
	SAFE_DELETE_ARRAY(vertices);

	//Begining of instanceing

	m_instance = new InstanceType[m_instanceCount];
	if (!m_instance)
		return false;

	instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	instanceBufferDesc.ByteWidth = sizeof(InstanceType)* m_instanceCount;
	instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	instanceBufferDesc.MiscFlags = 0;
	instanceBufferDesc.StructureByteStride = 0;

	instanceData.pSysMem = m_instance;
	instanceData.SysMemPitch = 0;
	instanceData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&instanceBufferDesc, &instanceData, &m_instanceBuffer);
	if (FAILED(result))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srv;
	ZeroMemory(&srv, sizeof(srv));
	m_texture->GetTexture()->GetDesc(&srv);

	memset(m_instance, 0, (sizeof(InstanceType) * m_instanceCount));

	//  UAV Stuff
	//D3D11_TEXTURE2D_DESC textureDesc;
	//ZeroMemory(&textureDesc, sizeof(textureDesc));
	//textureDesc.Width = 100;
	//textureDesc.Height = 95;
	//textureDesc.MipLevels = 1;
	//textureDesc.ArraySize = 1;
	//textureDesc.SampleDesc.Count = 1;
	//textureDesc.Usage = D3D11_USAGE_DEFAULT;
	//textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	//textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//textureDesc.CPUAccessFlags = 0;
	//textureDesc.MiscFlags = 0;

	//result = device->CreateTexture2D(&textureDesc, NULL, &m_uavTex);
	//if (FAILED(result))
	//	return false;

	//D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	//ZeroMemory(&uavDesc, sizeof(uavDesc));
	//uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	//uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	//uavDesc.Texture2D.MipSlice = 0;

	//result = device->CreateUnorderedAccessView(m_uavTex, &uavDesc, &m_uav);
	//if (FAILED(result))
	//	return false;

	return true;
}

void ParticleSystem::EmitParticles(float delta)
{
	if (m_particlePerSecond <= 0)
		return;
	m_accumulatedTime += delta;

	while (m_accumulatedTime > ( 1.0f / m_particlePerSecond))
	{
		m_accumulatedTime -= 1.0f / m_particlePerSecond;
		SpawnParticle();
	}
}

void ParticleSystem::SpawnParticle()
{
	bool found;
	float positionX, positionY, positionZ, velocity, red, green, blue;
	int index, i, j;
	if (m_currentParticleCount < m_maxParticles - 1)
	{
		positionX = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationX;
		positionY = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationY + 30;
		positionZ = (((float)rand() - (float)rand()) / RAND_MAX) * m_particleDeviationZ;

		m_particleList[m_currentParticleCount].initialPos = XMFLOAT3(positionX, positionY, positionZ);
		m_particleList[m_currentParticleCount].initialVel = XMFLOAT3(0.0f, -1.0f, 0.0f);
		m_particleList[m_currentParticleCount].age = 0.0f;
		m_particleList[m_currentParticleCount].offset = (float)rand() / RAND_MAX;
		++m_currentParticleCount;
	}
}

void ParticleSystem::UpdateParticles(float delta)
{
	for (int i = 0; i < m_currentParticleCount; ++i)
	{
		m_particleList[i].age += delta;
		
		if (m_particleList[i].age > 30.0f)
		{
			std::swap(m_particleList[i], m_particleList[m_currentParticleCount - 1]);
			--m_currentParticleCount;
		}
	}
}
void ParticleSystem::CalculateInstancePositions(int begin, int end)
{
	float x, y, z, offset, yAmplitude, zAmplitude, age, yVelocity;
	XMFLOAT3 initialPos;
	for (int i = begin; i < end; ++i)
	{
		offset = m_particleList[i].offset;
		yAmplitude = 0.5f;
		zAmplitude = 0.5f;
		age = m_particleList[i].age;
		initialPos = m_particleList[i].initialPos;
		yVelocity = m_particleList[i].initialVel.y;

		x = yAmplitude * sin(age * 1 * offset);
		x += yAmplitude * sin(age * 0.5f * offset);
		x += initialPos.x;
		
		y = zAmplitude * sin(age * 0.5f * offset);
		y += zAmplitude * sin(age * 0.66f * offset);
		y += age * yVelocity + initialPos.y;

		z = zAmplitude * sin(age * 0.66f * offset);
		z += zAmplitude * sin(age * 0.4f * offset);
		z += initialPos.z;

		m_instance[i].position = XMFLOAT3(x, y, z);
		m_instance[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

bool ParticleSystem::UpdateBuffers(ID3D11DeviceContext *deviceContext)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	InstanceType* instancePtr;

	int totalThreads = 8;
	
	float calculationsPerThread = (float)m_currentParticleCount / (float)totalThreads;
	
	for (int i = 0; i < totalThreads; ++i)
	{
		m_threads.push_back(std::thread(&ParticleSystem::CalculateInstancePositions, this, (int)((float)i * calculationsPerThread), (int)((float)(i + 1) * calculationsPerThread)));
	}
	for (int i = 0; i < m_threads.size(); ++i)
	{
		m_threads[i].join();
	}
	m_threads.clear();
	//CalculateInstancePositions(0, m_currentParticleCount);

	result = deviceContext->Map(m_instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	instancePtr = (InstanceType*)mappedResource.pData;
	memcpy(instancePtr, (void*)m_instance, (sizeof(InstanceType) * m_instanceCount));
	deviceContext->Unmap(m_instanceBuffer, 0);

	return true;
}

void ParticleSystem::RenderBuffers(ID3D11DeviceContext *deviceContext)
{
	unsigned int strides[2];
	unsigned int offsets[2];
	ID3D11Buffer* bufferPointers[2];

	strides[0] = sizeof(VertexType);
	strides[1] = sizeof(InstanceType);

	offsets[0] = 0;
	offsets[1] = 0;

	bufferPointers[0] = m_vertexBuffer;
	bufferPointers[1] = m_instanceBuffer;

	deviceContext->IASetVertexBuffers(0, 2, bufferPointers, strides, offsets);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}