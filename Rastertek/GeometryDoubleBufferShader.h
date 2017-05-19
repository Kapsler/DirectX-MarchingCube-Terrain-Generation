#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>

class GeometryDoubleBufferShader
{
public:
	GeometryDoubleBufferShader();
	~GeometryDoubleBufferShader();

	bool Initialize(ID3D11Device* device, WCHAR* filename, D3D11_BUFFER_DESC bufferDesc, D3D11_SO_DECLARATION_ENTRY* declarationEntry, UINT declarationEntryCount);
	void Set(ID3D11DeviceContext* context);
	void BufferSwap();

	ID3D11GeometryShader* geometryShader = nullptr;
	ID3D11Buffer *inputBuffer = nullptr, *outputBuffer = nullptr;

};

