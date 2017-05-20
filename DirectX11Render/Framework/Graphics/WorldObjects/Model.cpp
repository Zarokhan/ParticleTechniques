#include "Model.h"

Model::Model()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_Texture = 0;
	m_NormalTexture = 0;
	m_model = 0;

	// Math
	m_worldMatrix = XMMatrixIdentity();
	m_position = XMFLOAT3(0.f, 0.f, 0.f);
	m_scale = XMFLOAT3(1.f, 1.f, 1.f);
	m_rotation = XMFLOAT3(0.f, 0.f, 0.f);

	m_textureScale = 1.0f;
}

Model::Model(const Model &)
{
}

Model::~Model()
{
	SAFE_RELEASE(m_indexBuffer);
	SAFE_RELEASE(m_vertexBuffer);
	SAFE_DELETE(m_Texture);
	SAFE_DELETE(m_NormalTexture);
	SAFE_DELETE_ARRAY(m_model);
}

bool Model::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	vertices = new VertexType[m_indexCount];
	if (!vertices)
		return false;

	indices = new unsigned long[m_indexCount];
	if (!indices)
		return false;

	for (int i = 0; i < m_indexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);
		vertices[i].tangent = XMFLOAT3(m_model[i].tx, m_model[i].ty, m_model[i].tz);
		vertices[i].binormal = XMFLOAT3(m_model[i].bx, m_model[i].by, m_model[i].bz);

		indices[i] = i;
	}

	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_indexCount;
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

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
	return true;
}

bool Model::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const wchar_t* texPath, const wchar_t* normalPath)
{
	bool result;

	m_Texture = new Texture;
	if (!m_Texture)
		return false;

	result = m_Texture->Initialize(device, deviceContext, texPath);
	if (!result)
		return false;

	m_NormalTexture = new Texture;
	if (!m_NormalTexture)
		return false;

	result = m_NormalTexture->Initialize(device, deviceContext, normalPath);
	if (!result)
		return false;

	return true;
}

void Model::CalculateModelVectors()
{
	int faceCount, index;
	TempVertexType vertex1, vertex2, vertex3;
	VectorType tangent, binormal, normal;

	faceCount = m_indexCount / 3;
	index = 0;

	for (int i = 0; i < faceCount; i++)
	{
		vertex1.x = m_model[index].x;
		vertex1.y = m_model[index].y;
		vertex1.z = m_model[index].z;
		vertex1.tu = m_model[index].tu;
		vertex1.tv = m_model[index].tv;
		vertex1.nx = m_model[index].nx;
		vertex1.ny = m_model[index].ny;
		vertex1.nz = m_model[index].nz;
		index++;

		vertex2.x = m_model[index].x;
		vertex2.y = m_model[index].y;
		vertex2.z = m_model[index].z;
		vertex2.tu = m_model[index].tu;
		vertex2.tv = m_model[index].tv;
		vertex2.nx = m_model[index].nx;
		vertex2.ny = m_model[index].ny;
		vertex2.nz = m_model[index].nz;
		index++;

		vertex3.x = m_model[index].x;
		vertex3.y = m_model[index].y;
		vertex3.z = m_model[index].z;
		vertex3.tu = m_model[index].tu;
		vertex3.tv = m_model[index].tv;
		vertex3.nx = m_model[index].nx;
		vertex3.ny = m_model[index].ny;
		vertex3.nz = m_model[index].nz;
		index++;

		CalculateTangentBinormal(vertex1, vertex2, vertex3, tangent, binormal);
		CalculateNormal(tangent, binormal, normal);

		m_model[index - 1].nx = normal.x;
		m_model[index - 1].ny = normal.y;
		m_model[index - 1].nz = normal.z;
		m_model[index - 1].tx = tangent.x;
		m_model[index - 1].ty = tangent.y;
		m_model[index - 1].tz = tangent.z;
		m_model[index - 1].bx = binormal.x;
		m_model[index - 1].by = binormal.y;
		m_model[index - 1].bz = binormal.z;

		m_model[index - 2].nx = normal.x;
		m_model[index - 2].ny = normal.y;
		m_model[index - 2].nz = normal.z;
		m_model[index - 2].tx = tangent.x;
		m_model[index - 2].ty = tangent.y;
		m_model[index - 2].tz = tangent.z;
		m_model[index - 2].bx = binormal.x;
		m_model[index - 2].by = binormal.y;
		m_model[index - 2].bz = binormal.z;

		m_model[index - 3].nx = normal.x;
		m_model[index - 3].ny = normal.y;
		m_model[index - 3].nz = normal.z;
		m_model[index - 3].tx = tangent.x;
		m_model[index - 3].ty = tangent.y;
		m_model[index - 3].tz = tangent.z;
		m_model[index - 3].bx = binormal.x;
		m_model[index - 3].by = binormal.y;
		m_model[index - 3].bz = binormal.z;
	}
}

void Model::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
	VectorType vector1, vector2;
	float tuVector[2], tvVector[2];
	float den;
	float lenght;

	vector1.x = vertex2.x - vertex1.x;
	vector1.y = vertex2.y - vertex1.y;
	vector1.z = vertex2.z - vertex1.z;

	vector2.x = vertex3.x - vertex1.x;
	vector2.y = vertex3.y - vertex1.y;
	vector2.z = vertex3.z - vertex1.z;

	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tvVector[1] = vertex3.tv - vertex1.tv;
	tuVector[1] = vertex3.tu - vertex1.tu;

	den = 1.f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	tangent.x = (tvVector[1] * vector1.x - tvVector[0] * vector2.x) * den;
	tangent.y = (tvVector[1] * vector1.y - tvVector[0] * vector2.y) * den;
	tangent.z = (tvVector[1] * vector1.z - tvVector[0] * vector2.z) * den;

	binormal.x = (tuVector[0] * vector2.x - tuVector[1] * vector1.x) * den;
	binormal.y = (tuVector[0] * vector2.y - tuVector[1] * vector1.y) * den;
	binormal.z = (tuVector[0] * vector2.z - tuVector[1] * vector1.z) * den;

	lenght = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize
	tangent.x = tangent.x / lenght;
	tangent.y = tangent.y / lenght;
	tangent.z = tangent.z / lenght;

	lenght = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize
	binormal.x = binormal.x / lenght;
	binormal.y = binormal.y / lenght;
	binormal.z = binormal.z / lenght;
}

void Model::CalculateNormal(VectorType tangent, VectorType binormal, VectorType& normal)
{
	float lenght;

	normal.x = (tangent.y * binormal.z) - (tangent.z * binormal.y);
	normal.y = (tangent.z * binormal.x) - (tangent.x * binormal.z);
	normal.z = (tangent.x * binormal.y) - (tangent.y * binormal.x);

	lenght = sqrt((normal.x * normal.x) + (normal.y * normal.y) + (normal.z * normal.z));

	normal.x = normal.x / lenght;
	normal.y = normal.y / lenght;
	normal.z = normal.z / lenght;
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
	m_worldMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z) * XMMatrixRotationX(m_rotation.x) * XMMatrixRotationY(m_rotation.y) * XMMatrixRotationZ(m_rotation.z) * XMMatrixTranslation(m_position.x, m_position.y, m_position.z);
	
	RenderBuffers(deviceContext);
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}