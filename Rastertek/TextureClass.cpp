#include "TextureClass.h"

TextureClass::TextureClass()
{
	textures[0] = nullptr;
	textureViews[0] = nullptr;
	textures[1] = nullptr;
	textureViews[1] = nullptr;
}

TextureClass::TextureClass(const TextureClass&)
{
}

TextureClass::~TextureClass()
{
}

bool TextureClass::Initialize(ID3D11Device* device, WCHAR* filename1, WCHAR* filename2)
{
	HRESULT result;

	result = CreateDDSTextureFromFile(device, filename1, &textures[0], &textureViews[0]);
	if (FAILED(result))
	{
		return false;
	}

	result = CreateDDSTextureFromFile(device, filename2, &textures[1], &textureViews[1]);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void TextureClass::Shutdown()
{
	if(textureViews[0])
	{
		textureViews[0]->Release();
		textureViews[0] = nullptr;
	}

	if (textures[0])
	{
		textures[0]->Release();
		textures[0] = nullptr;
	}

	if(textureViews[1])
	{
		textureViews[1]->Release();
		textureViews[1] = nullptr;
	}

	if (textures[1])
	{
		textures[1]->Release();
		textures[1] = nullptr;
	}
}

ID3D11Resource** TextureClass::GetTextureArray()
{
	return textures;
}

ID3D11ShaderResourceView** TextureClass::GetTextureViewArray()
{
	return textureViews;
}
