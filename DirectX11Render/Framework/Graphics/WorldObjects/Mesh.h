#ifndef _MESH_H_
#define _MESH_H_

//////////////
// INCLUDES //
//////////////
#include <assimp\Importer.hpp>
#include <assimp\postprocess.h>
#include <assimp\mesh.h>
#include <assimp\scene.h>
#include <vector>
#include <string>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "Model.h"

////////////////////////////////////////////////////////////////////////////////
// Class name: Mesh
////////////////////////////////////////////////////////////////////////////////
class Mesh: public Model
{
public:
	Mesh();
	Mesh(const Mesh&);
	~Mesh();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const std::string&, std::wstring* textureName, std::wstring* textureFormat);

private:
	bool LoadModel();

private:
	std::string m_path;
	wchar_t m_texPath;
};

#endif