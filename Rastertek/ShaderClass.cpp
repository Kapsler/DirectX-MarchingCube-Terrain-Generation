#include "ShaderClass.h"
#include <SimpleMath.h>

ShaderClass::ShaderClass()
{
	vertexShader = nullptr;
	geometryShader = nullptr;
	pixelShader = nullptr;
	layout = nullptr;
	matrixBuffer = nullptr;
	sampleStateWrap = nullptr;
	sampleStateClamp = nullptr;
}

ShaderClass::ShaderClass(const ShaderClass&)
{
}

ShaderClass::~ShaderClass()
{
}

bool ShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	result = InitializeShader(device, hwnd, L"VertexShader.hlsl", L"GeometryShader.hlsl", L"PixelShader.hlsl");
	if(!result)
	{
		return false;
	}

	return true;
}

void ShaderClass::Shutdown()
{
	ShutdownShader();
}

bool ShaderClass::Render(ID3D11DeviceContext* context, int indexCount, int instanceCount,
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX	projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp)
{
	bool result;

	result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, eyePos, eyeDir, eyeUp);
	if(!result)
	{
		return false;
	}

	RenderShader(context, indexCount, instanceCount);

	return true;
}

bool ShaderClass::Render(ID3D11DeviceContext* context, int vertexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp)
{
	bool result;

	result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, eyePos, eyeDir, eyeUp);
	if (!result)
	{
		return false;
	}

	RenderShader(context, vertexCount);

	return true;
}

bool ShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vertexFilename, WCHAR* geometryFilename, WCHAR* pixelFilename)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* geometryShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc, eyeBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	DWORD shaderflags = 
		D3DCOMPILE_ENABLE_STRICTNESS 
	| D3DCOMPILE_DEBUG 
	| D3DCOMPILE_SKIP_OPTIMIZATION
	;

	errorMessage = nullptr;
	vertexShaderBuffer = nullptr;
	pixelShaderBuffer = nullptr;

	//Compile vertex shader
	result = D3DCompileFromFile(vertexFilename, nullptr, nullptr, "main", "vs_5_0", shaderflags, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vertexFilename);
		}
		else
		{
			MessageBox(hwnd, vertexFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	//Compile geometry shader
	result = D3DCompileFromFile(geometryFilename, nullptr, nullptr, "main", "gs_5_0", shaderflags, 0, &geometryShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vertexFilename);
		}
		else
		{
			MessageBox(hwnd, vertexFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	//Compile Pixel Shader
	result = D3DCompileFromFile(pixelFilename, nullptr, nullptr, "main", "ps_5_0", shaderflags, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, pixelFilename);
		}
		else
		{
			MessageBox(hwnd, pixelFilename, L"Missing Shader File", MB_OK);
		}

		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), nullptr, &vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreateGeometryShader(geometryShaderBuffer->GetBufferPointer(), geometryShaderBuffer->GetBufferSize(), nullptr, &geometryShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), nullptr, &pixelShader);
	if(FAILED(result))
	{
		return false;
	}

	//Vertex Input Layout Description
	//needs to mach VertexInputType
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &layout);
	if(FAILED(result))
	{
		return false;
	}

	//Release Buffer which are no longer needed
	vertexShaderBuffer->Release();
	vertexShaderBuffer = nullptr;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = nullptr;

	//Create wrap sampler state desc
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create wrap sampler state
	result = device->CreateSamplerState(&samplerDesc, &sampleStateWrap);
	if(FAILED(result))
	{
		return false;
	}

	//Create clamp sampler state desc
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	//Create wrap sampler state
	result = device->CreateSamplerState(&samplerDesc, &sampleStateClamp);
	if(FAILED(result))
	{
		return false;
	}

	//Setup matrix Buffer Description
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	//Make Buffer accessible
	result = device->CreateBuffer(&matrixBufferDesc, nullptr, &matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//Setup Eye Buffer Description
	eyeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	eyeBufferDesc.ByteWidth = sizeof(EyeBufferType);
	eyeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	eyeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	eyeBufferDesc.MiscFlags = 0;
	eyeBufferDesc.StructureByteStride = 0;

	//Make Buffer accessible
	result = device->CreateBuffer(&eyeBufferDesc, nullptr, &eyeBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void ShaderClass::ShutdownShader()
{
	if(sampleStateWrap)
	{
		sampleStateWrap->Release();
		sampleStateWrap = nullptr;
	}

	if(sampleStateClamp)
	{
		sampleStateClamp->Release();
		sampleStateClamp = nullptr;
	}

	if(matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = nullptr;
	}
	if(layout)
	{
		layout->Release();
		layout = nullptr;
	}
	if (pixelShader)
	{
		pixelShader->Release();
		pixelShader = nullptr;
	}
	if (geometryShader)
	{
		geometryShader->Release();
		geometryShader = nullptr;
	}
	if(vertexShader)
	{
		vertexShader->Release();
		vertexShader = nullptr;
	}
}

void ShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize;
	ofstream fout;

	compileErrors = static_cast<char*>(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();

	fout.open("shader-error.txt");
	for(auto i = 0; i<bufferSize; i++)
	{
		fout << compileErrors;
	}
	fout.close();

	errorMessage->Release();
	errorMessage = nullptr;

	MessageBox(hwnd, L"Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);

}

bool ShaderClass::SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixData;
	EyeBufferType* eyeData;

	//DirectX11 need matrices transposed!
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// ### MATRIXBUFFER ###
	//Lock Buffer
	result = context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if(FAILED(result))
	{
		return false;
	}

	//Get pointer to data
	matrixData = static_cast<MatrixBufferType*>(mappedResource.pData);
	
	//Set data
	matrixData->world = worldMatrix;
	matrixData->view = viewMatrix;
	matrixData->projection = projectionMatrix;

	context->Unmap(matrixBuffer, 0);

	// ### EyeBuffer ###
	//Lock Buffer
	result = context->Map(eyeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	//Get pointer to data
	eyeData = static_cast<EyeBufferType*>(mappedResource.pData);

	//Set data
	eyeData->position = eyePos;
	eyeData->forward = eyeDir;
	eyeData->up = eyeUp;

	context->Unmap(eyeBuffer, 0);

	//Set constant buffer
	context->VSSetConstantBuffers(0, 1, &matrixBuffer);
	context->GSSetConstantBuffers(0, 1, &matrixBuffer);
	//context->GSSetConstantBuffers(2, 1, &eyeBuffer);
	context->PSSetConstantBuffers(0, 1, &matrixBuffer);

	return true;
}

void ShaderClass::RenderShader(ID3D11DeviceContext* context, int vertexCount)
{
	//Vertex Input Layout
	context->IASetInputLayout(layout);

	//Set Shaders
	context->VSSetShader(vertexShader, nullptr, 0);
	context->GSSetShader(geometryShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);

	//Set sample States
	context->PSSetSamplers(0, 1, &sampleStateWrap);
	context->PSSetSamplers(1, 1, &sampleStateClamp);

	context->Draw(vertexCount, 0);
}

void ShaderClass::RenderShader(ID3D11DeviceContext* context, int indexCount, int instanceCount)
{
	//Vertex Input Layout
	context->IASetInputLayout(layout);

	//Set Shaders
	context->VSSetShader(vertexShader, nullptr, 0);
	context->GSSetShader(geometryShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);

	//Set sample States
	context->PSSetSamplers(0, 1, &sampleStateWrap);
	context->PSSetSamplers(1, 1, &sampleStateClamp);

	//Render indices
	context->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}
