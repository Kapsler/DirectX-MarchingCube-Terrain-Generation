#pragma once

#include <d3d11.h>
#include "DDSTextureLoader.h"
using namespace DirectX;

class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	bool Initialize(ID3D11Device*, WCHAR*, WCHAR*);
	void Shutdown();

	ID3D11Resource** GetTextureArray();
	ID3D11ShaderResourceView** GetTextureViewArray();

private:
	ID3D11Resource* textures[2];
	ID3D11ShaderResourceView* textureViews[2];
};