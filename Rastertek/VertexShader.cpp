#include "VertexShader.h"
#include "ShaderUtility.h"

VertexShader::VertexShader()
{
	vertexShader = nullptr;
}

VertexShader::~VertexShader()
{
	if(vertexShader)
	{
		vertexShader->Release();
		vertexShader = nullptr;
	}
	if(layout)
	{
		layout->Release();
		layout = nullptr;
	}
}

bool VertexShader::Initialize(ID3D11Device* device, WCHAR* filename, D3D11_INPUT_ELEMENT_DESC* polygonLayout, UINT layoutCount)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* vertexShaderBuffer;

	DWORD shaderflags =
		D3DCOMPILE_ENABLE_STRICTNESS
	|	D3DCOMPILE_DEBUG
	|	D3DCOMPILE_SKIP_OPTIMIZATION
	;

	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;

	//Compile VS
	result = D3DCompileFromFile(filename, nullptr, nullptr, "main", "vs_5_0", shaderflags, 0, &vertexShaderBuffer, &errorMessage);
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
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateInputLayout(polygonLayout, layoutCount, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &layout);
	if (FAILED(result))
	{
		return false;
	}

	//Release Buffer which are no longer needed
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	return true;
}

void VertexShader::Set(ID3D11DeviceContext* context)
{
	//Vertex Input Layout
	context->IASetInputLayout(layout);

	//Setting shader
	context->VSSetShader(vertexShader, nullptr, 0);
}
