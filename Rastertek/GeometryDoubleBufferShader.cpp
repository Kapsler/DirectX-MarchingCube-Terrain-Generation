#include "GeometryDoubleBufferShader.h"
#include "ShaderUtility.h"

GeometryDoubleBufferShader::GeometryDoubleBufferShader()
{

}

GeometryDoubleBufferShader::~GeometryDoubleBufferShader()
{
	if (inputBuffer)
	{
		inputBuffer->Release();
		inputBuffer = nullptr;
	}

	if (outputBuffer)
	{
		outputBuffer->Release();
		outputBuffer = nullptr;
	}

	if (geometryShader)
	{
		geometryShader->Release();
		geometryShader = nullptr;
	}
}

bool GeometryDoubleBufferShader::Initialize(ID3D11Device* device, WCHAR* filename, D3D11_BUFFER_DESC bufferDesc, D3D11_SO_DECLARATION_ENTRY* declarationEntry, UINT declarationEntryCount)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* geometryShaderBuffer;

	DWORD shaderflags =
		D3DCOMPILE_ENABLE_STRICTNESS
		| D3DCOMPILE_DEBUG
		| D3DCOMPILE_SKIP_OPTIMIZATION
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

	//Input Buffer
	{

		result = device->CreateBuffer(&bufferDesc, nullptr, &inputBuffer);
		if (FAILED(result))
		{
			return result;
		}
	}
	//Output Buffer
	{

		result = device->CreateBuffer(&bufferDesc, nullptr, &outputBuffer);
		if (FAILED(result))
		{
			return result;
		}
	}

	return true;
}

void GeometryDoubleBufferShader::Set(ID3D11DeviceContext* context)
{
	//Setting shader
	context->GSSetShader(geometryShader, nullptr, 0);
}
