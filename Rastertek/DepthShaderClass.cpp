#include "DepthShaderClass.h"

DepthShaderClass::DepthShaderClass()
{
	vertexShader = nullptr;
	pixelShader = nullptr;
	layout = nullptr;
	matrixBuffer = nullptr;
}

DepthShaderClass::DepthShaderClass(const DepthShaderClass&)
{
}

DepthShaderClass::~DepthShaderClass()
{
}

bool DepthShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	result = InitializeShader(device, hwnd, L"Depth_VS.hlsl", L"Depth_PS.hlsl");
	if(!result)
	{
		return false;
	}

	return true;
}

void DepthShaderClass::Shutdown()
{
	ShutdownShader();
}

bool DepthShaderClass::Render(ID3D11DeviceContext* context, int indexCount, int instanceCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX	projectionMatrix)
{
	bool result;

	result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix);
	if(!result)
	{
		return false;
	}

	RenderShader(context, indexCount, instanceCount);

	return true;
}

bool DepthShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vertexFilename, WCHAR* pixelFilename)
{
	HRESULT result;
	ID3DBlob* errorMessage;
	ID3DBlob* vertexShaderBuffer;
	ID3DBlob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[1];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	DWORD shaderflags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

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
	if(FAILED(result))
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

	return true;
}

void DepthShaderClass::ShutdownShader()
{
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
	if(pixelShader)
	{
		pixelShader->Release();
		pixelShader = nullptr;
	}
	if(vertexShader)
	{
		vertexShader->Release();
		vertexShader = nullptr;
	}
}

void DepthShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
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

bool DepthShaderClass::SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

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
	dataPtr = static_cast<MatrixBufferType*>(mappedResource.pData);
	
	//Set data
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	context->Unmap(matrixBuffer, 0);
	bufferNumber = 0;

	//Set constant buffer
	context->VSSetConstantBuffers(bufferNumber, 1, &matrixBuffer);

	return true;
}

void DepthShaderClass::RenderShader(ID3D11DeviceContext* context, int indexCount, int instanceCount)
{
	//Vertex Input Layout
	context->IASetInputLayout(layout);

	//Set Shaders
	context->VSSetShader(vertexShader, nullptr, 0);
	context->PSSetShader(pixelShader, nullptr, 0);

	//Render indices
	context->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}
