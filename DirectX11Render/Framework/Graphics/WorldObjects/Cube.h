#ifndef _CUBE_H_
#define _CUBE_H_

//////////////
// INCLUDES //
//////////////
#include <fstream>

///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "Model.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Class name: Cube
////////////////////////////////////////////////////////////////////////////////
class Cube: public Model
{
public:
	Cube();
	Cube(const Cube&);
	~Cube();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, char*, std::wstring* textureName, std::wstring* textureFormat);

private:
	bool LoadModel(char*);
	void Zoom(const float& dt);
};

#endif