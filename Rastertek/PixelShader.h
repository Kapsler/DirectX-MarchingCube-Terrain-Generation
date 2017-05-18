#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class PixelShader
{
public:
	PixelShader();
	~PixelShader();

	bool Initialize(ID3D11Device* device, WCHAR* filename);
	void Set(ID3D11DeviceContext* context);

	ID3D11PixelShader* pixelShader;

};
