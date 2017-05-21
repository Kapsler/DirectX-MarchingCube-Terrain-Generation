#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class GeometryOutputShader
{
public:
	GeometryOutputShader();
	~GeometryOutputShader();

	bool Initialize(ID3D11Device* device, WCHAR* filename, D3D11_BUFFER_DESC bufferDesc, D3D11_SO_DECLARATION_ENTRY* declarationEntry, UINT declarationEntryCount);
	void Set(ID3D11DeviceContext* context);
	ID3D11Buffer* GetReadBuffer(ID3D11DeviceContext* context);

	ID3D11GeometryShader* geometryShader;
	ID3D11Buffer *outputBuffer;

private:
	ID3D11Buffer *readBuffer;

};
