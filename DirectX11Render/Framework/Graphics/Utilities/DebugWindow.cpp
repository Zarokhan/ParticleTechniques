#include "DebugWindow.h"

DebugWindow::DebugWindow()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}

DebugWindow::DebugWindow(const DebugWindow& other)
{
}

DebugWindow::~DebugWindow()
{
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);
}

bool DebugWindow::Initialize(ID3D11Device* device, int screenWidth, int screenHeight, int bitmapWidth, int bitmapHeight)
{
	bool result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_bitmapWidth = bitmapWidth;
	m_bitmapHeight = bitmapHeight;
	m_previousPosX = -1;
	m_previousPosY = -1;

	result = InitializeBuffers(device);
	if (!result)
		return false;

	return true;
}

bool DebugWindow::Render(ID3D11DeviceContext* deviceContext, int posX, int posY)
{
	bool result;

	result = UpdateBuffers(deviceContext, posX, posY);
	if (!result)
		return false;

	RenderBuffers(deviceContext);

	return true;
}

bool DebugWindow::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	m_vertexCount = 6;
	m_indexCount = m_vertexCount;

	vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	for (int i = 0; i < m_indexCount; i++)
		indices[i] = i;

	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
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

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);

	return true;
}

bool DebugWindow::UpdateBuffers(ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
	float left, right, top, bottom;
	VertexType* vertices, *verticesPtr;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result;

	if ((positionX == m_previousPosX) && (positionY == m_previousPosY))
		return true;

	m_previousPosX = positionX;
	m_previousPosY = positionY;

	left = (float)(m_screenWidth * 0.5f * -1) + (float)positionX;
	right = left + (float)m_bitmapWidth;
	top = (float)(m_screenHeight * 0.5f) - (float)positionY;
	bottom = top - (float)m_bitmapHeight;

	vertices = new VertexType[m_vertexCount];
	if (!vertices)
		return false;

	// First triangle
	vertices[0].position = XMFLOAT3(left, top, 0.f);
	vertices[0].texture = XMFLOAT2(0.f, 0.f);
	
	vertices[1].position = XMFLOAT3(right, bottom, 0.f);
	vertices[1].texture = XMFLOAT2(1.f, 1.f);
	
	vertices[2].position = XMFLOAT3(left, top, 0.f);
	vertices[2].texture = XMFLOAT2(0.f, 1.f);
	// Second
	vertices[3].position = XMFLOAT3(left, top, 0.f);
	vertices[3].texture = XMFLOAT2(0.f, 0.f);
	
	vertices[4].position = XMFLOAT3(right, top, 0.f);
	vertices[4].texture = XMFLOAT2(1.f, 0.f);
	
	vertices[5].position = XMFLOAT3(right, bottom, 0.f);
	vertices[5].texture = XMFLOAT2(1.f, 1.f);

	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return false;

	verticesPtr = (VertexType*)mappedResource.pData;

	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	deviceContext->Unmap(m_vertexBuffer, 0);

	SAFE_DELETE_ARRAY(vertices);

	return true;
}

void DebugWindow::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride, offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
