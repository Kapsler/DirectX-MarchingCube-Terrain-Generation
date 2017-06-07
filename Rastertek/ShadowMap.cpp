#include "ShadowMap.h"
#include "GraphicsClass.h"

ShadowMap::ShadowMap(ID3D11Device* device, int sampleCount, int qualityLevel)
{
	InitializeBuffers(device, sampleCount, qualityLevel);
	InitializeShaders(device);
}

ShadowMap::ShadowMap(const ShadowMap& other)
{
	//TODO Implement copy constructor
}

ShadowMap::~ShadowMap()
{
}

void ShadowMap::Prepare(ID3D11DeviceContext* deviceContext)
{
	//Set texture as render target
	shadowMapTexture->SetRenderTarget(deviceContext);

	//Clear rendertexture
	shadowMapTexture->ClearRenderTarget(deviceContext, 0.0f, 0.0f, 0.0f, 1.0f);
}

void ShadowMap::Render(ID3D11DeviceContext* deviceContext, const UINT& vertexCount, const XMMATRIX& worldMatrix, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix)
{
	SetBufferData(deviceContext, worldMatrix, lightViewMatrix, lightProjectionMatrix);

	//Set Shaders
	vs->Set(deviceContext);
	ps->Set(deviceContext);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	//Render indices
	deviceContext->Draw(vertexCount, 0);
}

ID3D11ShaderResourceView* ShadowMap::GetShaderResourceView()
{
	return shadowMapTexture->GetShaderResourceView();
}

bool ShadowMap::InitializeBuffers(ID3D11Device* device, int sampleCount, int qualityLevel)
{
	bool result;
	D3D11_BUFFER_DESC matrixBufferDesc;

	shadowMapTexture = new RenderTextureClass();

	result = shadowMapTexture->Initialize(device, GraphicsConfig::SHADOWMAP_WIDTH, GraphicsConfig::SHADOWMAP_HEIGHT, GraphicsConfig::SCREEN_DEPTH, GraphicsConfig::SCREEN_NEAR, sampleCount, qualityLevel);
	if (!result)
	{
		printf("Couldn't generate shadowMapTexture!");
		return false;
	}

	//MatrixBuffer
	{
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

bool ShadowMap::InitializeShaders(ID3D11Device* device)
{
	{
		D3D11_INPUT_ELEMENT_DESC polygonLayout[1];

		//Vertex Input Layout Description
		//needs to machShadowVertexInputType
		polygonLayout[0].SemanticName = "SV_POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		vs = new VertexShader();
		vs->Initialize(device, L"Depth_VS.hlsl", polygonLayout, numElements);
	}

	ps = new PixelShader();
	ps->Initialize(device, L"Depth_PS.hlsl");

	return true;
}

bool ShadowMap::SetBufferData(ID3D11DeviceContext* context, const XMMATRIX& worldMatrix, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixData;

	// ### MATRIXBUFFER ###
	//Lock Buffer
	result = context->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	//Get pointer to data
	matrixData = static_cast<MatrixBufferType*>(mappedResource.pData);

	//Set data
	matrixData->world = XMMatrixTranspose(worldMatrix);
	matrixData->view = XMMatrixTranspose(lightViewMatrix);
	matrixData->projection = XMMatrixTranspose(lightProjectionMatrix);

	context->Unmap(matrixBuffer, 0);

	return true;
}

