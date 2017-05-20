#include "Cube.h"

Cube::Cube()
{
}

Cube::Cube(const Cube& other)
{
}

Cube::~Cube()
{
}

bool Cube::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* fileName, std::wstring* textureName, std::wstring* textureFormat)
{
	bool result;

	result = LoadModel(fileName);
	if (!result)
		return false;

	CalculateModelVectors();

	result = InitializeBuffers(device);
	if (!result)
		return false;

	std::wstring *param1 = new std::wstring, *param2 = new std::wstring;

	*param1 = L"Assets/texturepack/" + *textureName + L"_diffuse." + *textureFormat;
	*param2 = L"Assets/texturepack/" + *textureName + L"_bump." + *textureFormat;

	result = LoadTextures(device, deviceContext, param1->c_str(), param2->c_str());
	if (!result)
		return false;

	return true;
}

void Cube::Zoom(const float & dt)
{
	XMVECTOR scaleVector = XMLoadFloat3(&m_scale);
	scaleVector *= 1.01f;
	XMStoreFloat3(&m_scale, scaleVector);
}

bool Cube::LoadModel(char* fileName)
{
	ifstream fin;
	char input;
	int i;

	fin.open(fileName);

	if (fin.fail())
		return false;

	fin.get(input);
	while (input != ':')
		fin.get(input);

	fin >> m_vertexCount;

	m_indexCount = m_vertexCount;

	m_model = new ModelType[m_vertexCount];
	if (!m_model)
		return false;

	fin.get(input);
	while (input != ':')
		fin.get(input);
	
	fin.get(input);
	fin.get(input);

	for (i = 0; i<m_vertexCount; i++)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	fin.close();
	return true;
}


