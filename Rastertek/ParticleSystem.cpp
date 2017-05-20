#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(ID3D11Device* device)
{
	InitializeShaders(device);
	InitializeBuffers(device);
	worldMatrix = DirectX::XMMatrixIdentity();
}

ParticleSystem::ParticleSystem(const ParticleSystem& other)
{

}

ParticleSystem::~ParticleSystem()
{

}


void ParticleSystem::InitializeShaders(ID3D11Device* device)
{
	//ParticleUpdate VS
	{
		D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

		//Vertex Input Layout Description - ParticleAttributes
		polygonLayout[0].SemanticName = "SV_POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "TYPE";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32_UINT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = 0;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		particleUpdateVS = new VertexShader();
		particleUpdateVS->Initialize(device, L"ParticleUpdate_VS.hlsl", polygonLayout, numElements);
	}

	//ParticleVisuals VS
	{
		D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

		//Vertex Input Layout Description - ParticleAttributes
		polygonLayout[0].SemanticName = "SV_POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "TYPE";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32_UINT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = 0;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		particleVisualsVS = new VertexShader();
		particleVisualsVS->Initialize(device, L"ParticleRender_VS.hlsl", polygonLayout, numElements);
	}

	//ParticleUpdate GS
	{
		
		D3D11_BUFFER_DESC bufferDesc;
		ZeroMemory(&bufferDesc, sizeof(bufferDesc));

		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		bufferDesc.ByteWidth = sizeof(ParticleAttributes) * 1000;

		D3D11_SO_DECLARATION_ENTRY declarationEntry[2];
		ZeroMemory(&declarationEntry, sizeof(declarationEntry));

		declarationEntry[0].ComponentCount = 4;
		declarationEntry[0].SemanticIndex = 0;
		declarationEntry[0].SemanticName = "SV_POSITION";
		declarationEntry[0].OutputSlot = 0;
		declarationEntry[0].StartComponent = 0;
		declarationEntry[0].Stream = 0;

		declarationEntry[1].ComponentCount = 1;
		declarationEntry[1].SemanticIndex = 0;
		declarationEntry[1].SemanticName = "TYPE";
		declarationEntry[1].OutputSlot = 0;
		declarationEntry[1].StartComponent = 0;
		declarationEntry[1].Stream = 0;

		UINT numElements = sizeof(declarationEntry) / sizeof(declarationEntry[0]);

		particleUpdateGS = new GeometryDoubleBufferShader();
		particleUpdateGS->Initialize(device, L"ParticleUpdate_GS.hlsl", bufferDesc, declarationEntry, numElements);
	}

	//ParticleVisuals GS
	{
		particleVisualsGS = new GeometryShader();
		particleVisualsGS->Initialize(device, L"ParticleRender_GS.hlsl");
	}

	//ParticleVisuals PS
	{
		ps = new PixelShader();
		ps->Initialize(device, L"Particle_PS.hlsl");
	}
}

bool ParticleSystem::InitializeBuffers(ID3D11Device* device)
{
	HRESULT result;

	// VertexBuffer
	{
		
		D3D11_BUFFER_DESC vertexBufferDesc;
		D3D11_SUBRESOURCE_DATA vertexData;
		ParticleAttributes* vertices;

		vertices = new ParticleAttributes[1];
		vertices[0].position = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		vertices[0].type = 0;

		// Set the number of vertices in the vertex array.
		UINT vertexCount = 1;

		// Set up the description of the static vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(ParticleAttributes) * vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &kickstartVertexBuffer);
		if (FAILED(result))
		{
			return false;
		}

		// Release the arrays now that the vertex and index buffers have been created and loaded.
		delete[] vertices;
		vertices = nullptr;
	}

	//Matrix Constant Buffer
	{
		D3D11_BUFFER_DESC matrixBufferDesc;

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
	}

	return true;
}

void ParticleSystem::Kickstart(ID3D11DeviceContext* context)
{
	//Make VertexBuffer with one Emitter Particle
	//Use as Input for very first Renderpass
	//Later interactive

	UINT offset = 0, stride = sizeof(ParticleAttributes);

	//Set Pixelshader
	particleUpdateVS->Set(context);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//First Renderpass for Update
	particleUpdateGS->Set(context);
	context->SOSetTargets(1, &particleUpdateGS->outputBuffer, &offset);
	context->IASetVertexBuffers(0, 1, &kickstartVertexBuffer, &stride, &offset);

	context->Draw(1, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->SOSetTargets(0, nullptr, nullptr);

	isWarmedUp = true;
}

void ParticleSystem::Render(ID3D11DeviceContext* context, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
	if (!isWarmedUp)
	{
		Kickstart(context);
	}

	SetBufferData(context, viewMatrix, projectionMatrix);
	FirstRenderPass(context);
	SecondRenderPass(context);

}

void ParticleSystem::SetBufferData(ID3D11DeviceContext* context, const DirectX::XMMATRIX& viewMatrix, const DirectX::XMMATRIX& projectionMatrix)
{

	// Matrix Buffer
	{
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MatrixBufferType* matrixData;

		//Lock Buffer
		context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

		//Get pointer to data
		matrixData = static_cast<MatrixBufferType*>(mappedResource.pData);

		//Set data
		matrixData->world = worldMatrix;
		matrixData->view = viewMatrix;
		matrixData->projection = projectionMatrix;

		context->Unmap(matrixBuffer, 0);
	}
}

void ParticleSystem::FirstRenderPass(ID3D11DeviceContext* context)
{
	UINT offset = 0, stride = sizeof(ParticleAttributes);

	//First Renderpass for Update
	particleUpdateVS->Set(context);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->VSSetConstantBuffers(0, 1, &matrixBuffer);

	particleUpdateGS->BufferSwap();
	particleUpdateGS->Set(context);

	context->SOSetTargets(1, &particleUpdateGS->outputBuffer, &offset);
	context->IASetVertexBuffers(0, 1, &particleUpdateGS->inputBuffer, &stride, &offset);

	context->DrawAuto();
	context->GSSetShader(nullptr, nullptr, 0);
	context->SOSetTargets(0, nullptr, nullptr);
}

void ParticleSystem::SecondRenderPass(ID3D11DeviceContext* context)
{
	UINT offset = 0, stride = sizeof(ParticleAttributes);

	//Second Renderpass for Visualization
	particleVisualsVS->Set(context);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	context->IASetVertexBuffers(0, 1, &particleUpdateGS->outputBuffer, &stride, &offset);
	context->VSSetConstantBuffers(0, 1, &matrixBuffer);

	particleVisualsGS->Set(context);
	ps->Set(context);

	context->DrawAuto();
	context->GSSetShader(nullptr, nullptr, 0);
}