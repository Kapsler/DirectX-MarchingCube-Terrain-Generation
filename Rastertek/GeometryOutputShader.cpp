#include "GeometryOutputShader.h"
#include "ShaderUtility.h"

GeometryOutputShader::GeometryOutputShader():
	geometryShader (nullptr), 
	outputBuffer(nullptr),
	readBuffer(nullptr)
{
}

GeometryOutputShader::~GeometryOutputShader()
{
	if(geometryShader)
	{
		geometryShader->Release();
		geometryShader = nullptr;
	}
	if (outputBuffer)
	{
		outputBuffer->Release();
		outputBuffer = nullptr;
	}
	if (readBuffer)
	{
		readBuffer->Release();
		readBuffer = nullptr;
	}
}

bool GeometryOutputShader::Initialize(ID3D11Device* device, WCHAR* filename, D3D11_BUFFER_DESC bufferDesc, D3D11_SO_DECLARATION_ENTRY* declarationEntry, UINT declarationEntryCount)
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
	{
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
		result =
			device->CreateGeometryShaderWithStreamOutput(
				geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), declarationEntry, declarationEntryCount,
				nullptr, 0, D3D11_SO_NO_RASTERIZED_STREAM, nullptr, &geometryShader);

		if (FAILED(result))
		{
			return false;
		}

		//Release Buffer which are no longer needed
		geometryShaderBuffer->Release();
		geometryShaderBuffer = nullptr;
	}

	//Output Buffer
	{

		result = device->CreateBuffer(&bufferDesc, nullptr, &outputBuffer);
		if (FAILED(result))
		{
			return result;
		}
	}

	//Read Buffer
	{
		bufferDesc.Usage = D3D11_USAGE_STAGING;
		bufferDesc.BindFlags = 0;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		result = device->CreateBuffer(&bufferDesc, nullptr, &readBuffer);
		if (FAILED(result))
		{
			return result;
		}
	}

	return true;
}

void GeometryOutputShader::Set(ID3D11DeviceContext* context)
{
	//Setting shader
	context->GSSetShader(geometryShader, nullptr, 0);
}

ID3D11Buffer* GeometryOutputShader::GetReadBuffer(ID3D11DeviceContext* context)
{
	context->CopyResource(readBuffer, outputBuffer);
	return readBuffer;
}
