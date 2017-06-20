#include "ShaderUtility.h"
#include "HullShader.h"

HullShader::HullShader()
{
	hullShader = nullptr;
}

HullShader::~HullShader()
{
	if(hullShader)
	{
		hullShader->Release();
		hullShader = nullptr;
	}
}

bool HullShader::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* hullShaderBuffer;

	DWORD shaderflags =
		D3DCOMPILE_ENABLE_STRICTNESS
	|	D3DCOMPILE_DEBUG
	|	D3DCOMPILE_SKIP_OPTIMIZATION
	;

	errorMessage = nullptr;
	hullShaderBuffer = nullptr;

	//Compile VS
	result = D3DCompileFromFile(filename, nullptr, nullptr, "main", "hs_5_0", shaderflags, 0, &hullShaderBuffer, &errorMessage);
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
	result = device->CreateHullShader(hullShaderBuffer->GetBufferPointer(), hullShaderBuffer->GetBufferSize(), nullptr, &hullShader);
	if (FAILED(result))
	{
		return false;
	}

	//Release Buffer which are no longer needed
	hullShaderBuffer->Release();
	hullShaderBuffer = nullptr;

	return true;
}

void HullShader::Set(ID3D11DeviceContext* context)
{
	//Setting shader
	context->HSSetShader(hullShader, nullptr, 0);
}
