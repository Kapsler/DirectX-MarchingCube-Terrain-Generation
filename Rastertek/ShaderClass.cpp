#include "ShaderClass.h"
#include <SimpleMath.h>

ShaderClass::ShaderClass()
{
	matrixBuffer = nullptr;
	sampleStateWrap = nullptr;
	sampleStateClamp = nullptr;
	vs = nullptr;
	ps = nullptr;
	gs = nullptr;
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
	XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX	projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp,
	int initialSteps, int refinementSteps, float depthfactor)
{
	bool result;

	result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, eyePos, eyeDir, eyeUp, initialSteps, refinementSteps, depthfactor);
	if(!result)
	{
		return false;
	}

	RenderShader(context, indexCount, instanceCount);

	return true;
}

bool ShaderClass::Render(ID3D11DeviceContext* context, int vertexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp,
	int initialSteps, int refinementSteps, float depthfactor)
{
	bool result;

	result = SetShaderParameters(context, worldMatrix, viewMatrix, projectionMatrix, eyePos, eyeDir, eyeUp, initialSteps, refinementSteps, depthfactor);
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
	
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	D3D11_BUFFER_DESC matrixBufferDesc, eyeBufferDesc, factorBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	
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

	vs = new VertexShader();
	vs->Initialize(device, vertexFilename, polygonLayout);

	ps = new PixelShader();
	ps->Initialize(device, pixelFilename);

	gs = new GeometryShader();
	gs->Initialize(device, geometryFilename);

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

	//Setup Eye Buffer Description
	factorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	factorBufferDesc.ByteWidth = sizeof(FactorBufferType);
	factorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	factorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	factorBufferDesc.MiscFlags = 0;
	factorBufferDesc.StructureByteStride = 0;

	//Make Buffer accessible
	result = device->CreateBuffer(&factorBufferDesc, nullptr, &factorBuffer);
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
	if (ps)
	{
		delete ps;
		ps = nullptr;
	}
	if (gs)
	{
		delete gs;
		gs = nullptr;
	}
	if(vs)
	{
		delete vs;
		vs = nullptr;
	}
}

bool ShaderClass::SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp, int initialSteps, int refinementSteps, float depthfactor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixData;
	EyeBufferType* eyeData;
	FactorBufferType* factorData;


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

	// ### FactorBuffer (Pixel Shader Displacement) ###
	//Lock Buffer
	result = context->Map(factorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	//Get pointer to data
	factorData = static_cast<FactorBufferType*>(mappedResource.pData);

	//Set data
	factorData->steps_initial = initialSteps;
	factorData->steps_refinement = refinementSteps;
	factorData->depth_factor = depthfactor;

	context->Unmap(factorBuffer, 0);

	//Set constant buffer
	context->VSSetConstantBuffers(0, 1, &matrixBuffer);
	context->GSSetConstantBuffers(0, 1, &matrixBuffer);
	context->PSSetConstantBuffers(0, 1, &matrixBuffer);
	context->PSSetConstantBuffers(2, 1, &eyeBuffer);
	context->PSSetConstantBuffers(3, 1, &factorBuffer);

	return true;
}

void ShaderClass::RenderShader(ID3D11DeviceContext* context, int vertexCount)
{
	//Set Shaders
	vs->Set(context);
	gs->Set(context);
	ps->Set(context);

	//Set sample States
	context->PSSetSamplers(0, 1, &sampleStateWrap);
	context->PSSetSamplers(1, 1, &sampleStateClamp);

	context->Draw(vertexCount, 0);
}

void ShaderClass::RenderShader(ID3D11DeviceContext* context, int indexCount, int instanceCount)
{
	//Set Shaders
	vs->Set(context);
	gs->Set(context);
	ps->Set(context);

	//Set sample States
	context->PSSetSamplers(0, 1, &sampleStateWrap);
	context->PSSetSamplers(1, 1, &sampleStateClamp);

	//Render indices
	context->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
}
