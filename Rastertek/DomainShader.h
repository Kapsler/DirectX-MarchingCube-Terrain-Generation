#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class DomainShader
{
public:
	DomainShader();
	~DomainShader();

	bool Initialize(ID3D11Device* device, WCHAR* filename);
	void Set(ID3D11DeviceContext* context);

private:

	ID3D11DomainShader* domainShader;

};
