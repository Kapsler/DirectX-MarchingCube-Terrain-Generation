#include "PixelShader.h"
#include "ShaderUtility.h"

PixelShader::PixelShader()
{
	pixelShader = nullptr;
}

PixelShader::~PixelShader()
{
	if(pixelShader)
	{
		pixelShader->Release();
		pixelShader = nullptr;
	}
}

bool PixelShader::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* pixelShaderBuffer;

	DWORD shaderflags =
		D3DCOMPILE_ENABLE_STRICTNESS
	|	D3DCOMPILE_DEBUG
	|	D3DCOMPILE_SKIP_OPTIMIZATION
	;

	errorMessage = nullptr;
	pixelShaderBuffer = nullptr;

	//Compile PS
	result = D3DCompileFromFile(filename, nullptr, nullptr, "main", "ps_5_0", shaderflags, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			ShaderUtility::OutputShaderErrorMessage(errorMessage, filename);
		}
		else
		{
			printf("Missing Shader File %ls", filename);
		}

		return false;
	}

	//Create Shader Blob
	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	//Release Buffer which are no longer needed
	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	return true;
}

void PixelShader::Set(ID3D11DeviceContext* context)
{
	//Setting shader
	context->PSSetShader(pixelShader, nullptr, 0);
}
