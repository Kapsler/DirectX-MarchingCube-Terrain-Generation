#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class VertexShader
{
public:
	VertexShader();
	~VertexShader();

	bool Initialize(ID3D11Device* device, WCHAR* filename, D3D11_INPUT_ELEMENT_DESC* polygonLayout);
	void Set(ID3D11DeviceContext* context);

private:

	ID3D11VertexShader* vertexShader;
	ID3D11InputLayout* layout;

};
