#include "GeometryShader.h"
#include "ShaderUtility.h"

GeometryShader::GeometryShader()
{
	geometryShader = nullptr;
}

GeometryShader::~GeometryShader()
{
	if(geometryShader)
	{
		geometryShader->Release();
		geometryShader = nullptr;
	}
}

bool GeometryShader::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* geometryShaderBuffer;

	DWORD shaderflags =
		D3DCOMPILE_ENABLE_STRICTNESS
	|	D3DCOMPILE_DEBUG
	|	D3DCOMPILE_SKIP_OPTIMIZATION
	;

	errorMessage = nullptr;
	geometryShaderBuffer = nullptr;

	//Compile PS
	result = D3DCompileFromFile(filename, nullptr, nullptr, "main", "gs_5_0", shaderflags, 0, &geometryShaderBuffer, &errorMessage);
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
	result = device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), nullptr, &geometryShader);
	if (FAILED(result))
	{
		return false;
	}

	//Release Buffer which are no longer needed
	geometryShaderBuffer->Release();
	geometryShaderBuffer = nullptr;

	return true;
}

void GeometryShader::Set(ID3D11DeviceContext* context)
{
	//Setting shader
	context->GSSetShader(geometryShader, nullptr, 0);
}
