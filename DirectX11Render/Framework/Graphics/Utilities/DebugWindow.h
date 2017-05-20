#ifndef _DEBUGWINDOW_H_
#define _DEBUGWINDOW_H_

#include <d3d11.h>
#include <directxmath.h>
#include "../../DirectX.h"

using namespace DirectX;

class DebugWindow
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

public:
	DebugWindow();
	DebugWindow(const DebugWindow&);
	~DebugWindow();

	bool Initialize(ID3D11Device*, int, int, int, int);
	bool Render(ID3D11DeviceContext*, int, int);

	inline int GetIndexCount() const { return m_indexCount; }

private:
	bool InitializeBuffers(ID3D11Device*);
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;
	int m_previousPosX, m_previousPosY;
};

#endif