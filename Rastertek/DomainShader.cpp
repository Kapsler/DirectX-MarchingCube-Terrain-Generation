#include "ShaderUtility.h"
#include "DomainShader.h"

DomainShader::DomainShader()
{
	domainShader = nullptr;
}

DomainShader::~DomainShader()
{
	if(domainShader)
	{
		domainShader->Release();
		domainShader = nullptr;
	}
}

bool DomainShader::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* domainShaderBuffer;

	DWORD shaderflags =
		D3DCOMPILE_ENABLE_STRICTNESS
	|	D3DCOMPILE_DEBUG
	|	D3DCOMPILE_SKIP_OPTIMIZATION
	;

	errorMessage = nullptr;
	domainShaderBuffer = nullptr;

	//Compile VS
	result = D3DCompileFromFile(filename, nullptr, nullptr, "main", "ds_5_0", shaderflags, 0, &domainShaderBuffer, &errorMessage);
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
	result = device->CreateDomainShader(domainShaderBuffer->GetBufferPointer(), domainShaderBuffer->GetBufferSize(), nullptr, &domainShader);
	if (FAILED(result))
	{
		return false;
	}

	//Release Buffer which are no longer needed
	domainShaderBuffer->Release();
	domainShaderBuffer = nullptr;

	return true;
}

void DomainShader::Set(ID3D11DeviceContext* context)
{
	//Setting shader
	context->DSSetShader(domainShader, nullptr, 0);
}
