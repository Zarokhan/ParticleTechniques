#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const Mesh &)
{
}

Mesh::~Mesh()
{
}

bool Mesh::Initialize(ID3D11Device *device, ID3D11DeviceContext* deviceContext, const std::string& modelPath, std::wstring* textureName, std::wstring* textureFormat)
{
	bool result;

	m_path = modelPath;

	result = LoadModel();
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

bool Mesh::LoadModel()
{
	Assimp::Importer importer;
	m_vertexCount = m_indexCount = 0;

	const aiScene* scene = importer.ReadFile(m_path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);
	if (!scene)
		return false;

	//aiMesh* mesh = scene->mMeshes[0];
	std::vector<ModelType> modeltype;
	std::vector<unsigned int> iMaterialIndices;
	int indexCount = 0;
	int vertexCount = 0;
	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[i];
		int iMeshFaces = mesh->mNumFaces;
		iMaterialIndices.push_back(mesh->mMaterialIndex);
		for (int j = 0; j < iMeshFaces; j++)
		{
			const aiFace& face = mesh->mFaces[j];
			indexCount += face.mNumIndices;
			for (int k = 0; k < 3; k++)
			{
				ModelType type;
				type.x = mesh->mVertices[face.mIndices[k]].x;
				type.y = mesh->mVertices[face.mIndices[k]].y;
				type.z = mesh->mVertices[face.mIndices[k]].z;

				type.tu = mesh->mTextureCoords[0][face.mIndices[k]].x;
				type.tv = mesh->mTextureCoords[0][face.mIndices[k]].y;

				type.nx = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]].x : 1;
				type.ny = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]].y : 1;
				type.nz = mesh->HasNormals() ? mesh->mNormals[face.mIndices[k]].z : 1;
				modeltype.push_back(type);
			}
		}
		vertexCount += mesh->mNumVertices;
	}

	for (int i = 0; i < scene->mNumMaterials; ++i)
	{
		int texIndex = 0;
		aiString path;

		aiReturn texFound = scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		while (texFound == AI_SUCCESS)
		{
			texIndex++;
			texFound = scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
		}
		m_texPath = (wchar_t)path.data;
	}

	// Get numbers of indices
	m_indexCount = indexCount;

	// Get number of vertices
	m_vertexCount = vertexCount;

	m_model = new ModelType[m_indexCount];
	if (!m_model)
		return false;

	int test = modeltype.size();
	// Set vertices
	for (int i = 0; i < modeltype.size(); i++)
	{
		m_model[i].x = modeltype[i].x;
		m_model[i].y = modeltype[i].y;
		m_model[i].z = modeltype[i].z;

		m_model[i].tu = modeltype[i].tu;
		m_model[i].tv = modeltype[i].tv;

		m_model[i].nx = modeltype[i].nx;
		m_model[i].ny = modeltype[i].ny;
		m_model[i].nz = modeltype[i].nz;

		//XMFLOAT3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		//XMFLOAT4 col(1.0f, 1.0f, 1.0f, 1.0f);
		//XMFLOAT3 nor(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		//XMFLOAT2 tex((*mesh->mTextureCoords)[i].x, (*mesh->mTextureCoords)[i].y);
		//XMFLOAT3 binor(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		//XMFLOAT3 tan(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		//vertices[i] = { pos, tex, nor };
	}

	return true;
}