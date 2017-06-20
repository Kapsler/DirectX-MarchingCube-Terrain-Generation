#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class HullShader
{
public:
	HullShader();
	~HullShader();

	bool Initialize(ID3D11Device* device, WCHAR* filename);
	void Set(ID3D11DeviceContext* context);

private:

	ID3D11HullShader* hullShader;

};
