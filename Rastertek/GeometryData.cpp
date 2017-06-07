#include "GeometryData.h"
#include "TriangleLUT.h"

GeometryData::GeometryData(unsigned width, unsigned height, unsigned depth, TerrainType::Enum type, ID3D11Device* device, ID3D11DeviceContext* deviceContext, KdTree* treeToUse)
	: m_width(width), m_height(height), m_depth(depth), tree(treeToUse)
{

	m_cubeSize = DirectX::XMFLOAT3(64.0f, 64.0f, 64.0f);
	//2.0f to decrease density
	m_cubeStep = DirectX::XMFLOAT3(2.0f / m_cubeSize.x, 2.0f / m_cubeSize.y, 2.0f / m_cubeSize.z);
	worldMatrix = DirectX::XMMatrixIdentity();
	m_data = new float[m_width*m_depth*m_height];

	std::mt19937 generator(static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
	m_noiseOffset = generator();

	switch (type)
	{
	case TerrainType::CUBE:
		GenerateCubeData();
		break;
	case TerrainType::SPHERE:
		GenerateSphereData();
		break;
	case TerrainType::PILLAR:
		GeneratePillarData();
		break;
	case TerrainType::NOISE:
		GenerateNoiseData();
		break;
	case TerrainType::BUMPY_SPHERE:
		GenerateBumpySphere();
		break;
	case TerrainType::HELIX:
		GenerateHelixStructure();
		break;
	}

	m_texDesc = CreateTextureDesc();
	m_subData = CreateSubresourceData();
	m_texture3D = CreateTexture(device, m_texDesc, m_subData);
	m_densityMap = CreateDensityShaderResource(device, m_texture3D);
	m_triangleLUT = CreateTriangleLUTShaderResource(device);
	m_densitySampler = CreateDensitySamplerState(device);
	InitializeShaders(device);
	LoadTextures(device);
	CreatePSSamplerStates(device, m_wrapSampler, m_clampSampler);
	InitializeBuffers(device);
	GenerateDecalDescriptionBuffer(device, deviceContext);
}

GeometryData::~GeometryData()
{

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}

	if (m_densityMap)
	{
		m_densityMap->Release();
		m_densityMap = nullptr;
	}

	if (m_texture3D)
	{
		m_texture3D->Release();
		m_texture3D = nullptr;
	}

	if (m_densitySampler)
	{
		m_densitySampler->Release();
		m_densitySampler = nullptr;
	}

	if (m_clampSampler)
	{
		m_clampSampler->Release();
		m_clampSampler = nullptr;
	}

	if (m_wrapSampler)
	{
		m_wrapSampler->Release();
		m_wrapSampler = nullptr;
	}

	for(size_t i = 0u; i < 3; ++i)
	{
		if(m_colorTextures[i] != nullptr)
		{
			m_colorTextures[i]->Shutdown();
		}
		m_colorTextures[i] = nullptr;
	}

	delete m_data;

	if (marchingCubeVS)
	{
		delete marchingCubeVS;
		marchingCubeVS = nullptr;
	}

	if (marchingCubeGSO)
	{
		delete marchingCubeGSO;
		marchingCubeGSO = nullptr;
	}

	if (triplanarDisplacementPS)
	{
		delete triplanarDisplacementPS;
		triplanarDisplacementPS = nullptr;
	}

	if (geometryVS)
	{
		delete geometryVS;
		geometryVS = nullptr;
	}
}

void GeometryData::GenerateCubeData()
{
	unsigned int width_offset = m_width / 4;
	unsigned int height_offset = m_height / 4;
	unsigned int depth_offset = m_depth / 4;

	size_t index = 0;

	for (size_t z = 0u; z < m_depth; ++z)
	{
		for (size_t y = 0u; y < m_height; ++y)
		{
			for (size_t x = 0u; x < m_width; ++x)
			{

				if (
					x >= 0 + width_offset && x <= m_width - width_offset
					&& y >= 0 + height_offset && y <= m_height - height_offset
					&& z >= 0 + depth_offset && z <= m_depth - depth_offset)
				{
					m_data[index] = 1.0f;

				}
				else
				{
					m_data[index] = -1.0f;
				}

				index++;

			}
		}
	}


}

float GeometryData::getDistance(const float& p1x, const float& p1y, const float& p1z, const float& p2x, const float& p2y, const float& p2z)
{
	float dx, dy, dz;

	dx = p2x - p1x;
	dy = p2y - p1y;
	dz = p2z - p1z;

	return sqrt(dx * dx + dy * dy + dz * dz);
}

float GeometryData::getDistance2D(const float& p1x, const float& p1y, const float& p2x, const float& p2y)
{
	float dx, dy;

	dx = p2x - p1x;
	dy = p2y - p1y;

	return sqrt(dx * dx + dy * dy);
}

void GeometryData::GenerateSphereData()
{
	DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(m_width / 2.0f, m_height / 2.0f, m_depth / 2.0f);

	size_t index = 0u;
	float maxDistance = m_width / 2.0f;

	for (UINT z = 0; z < m_depth; z++)
	{
		for (UINT y = 0; y < m_height; y++)
		{
			for (UINT x = 0; x < m_width; x++)
			{
				//Take distance's complement so the nearer to the center the bigger the density
				m_data[index] = 1.0f - (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), center.x, center.y, center.z) / maxDistance);;

				index++;
			}
		}
	}
}

void GeometryData::GeneratePillarData()
{

	size_t index = 0u;
	float maxDistance = m_width / 25.0f;

	for (UINT z = 0; z < m_depth; z++)
	{
		for (UINT y = 0; y < m_height; y++)
		{
			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(m_width / 2.0f, static_cast<float>(y), m_depth / 2.0f);

			for (UINT x = 0; x < m_width; x++)
			{
				//Take distance's complement so the nearer to the center the bigger the density
				m_data[index] = 1.0f - (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), center.x, center.y, center.z) / maxDistance);

				index++;
			}
		}

	}
}
void GeometryData::GenerateHelixStructure()
{

	size_t index = 0u;
	float maxDistance = m_width / 7.5f;


	for (UINT z = 0; z < m_depth; z++)
	{
		for (UINT y = 0; y < m_height; y++)
		{
			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(m_width / 2.0f, static_cast<float>(y), m_depth / 2.0f);
			DirectX::XMFLOAT3 pillar1 = DirectX::XMFLOAT3(center.x + 10.0f * (float)sin(y / 7.0f), center.y, center.z + 10.0f * (float)cos(y / 7.0f));
			DirectX::XMFLOAT3 pillar2 = DirectX::XMFLOAT3(center.x + 10.0f * (float)sin(y / 7.0f + DirectX::XM_PI * 0.66f), center.y, center.z + 10.0f * (float)cos(y / 7.0f + DirectX::XM_PI * 0.66f));
			DirectX::XMFLOAT3 pillar3 = DirectX::XMFLOAT3(center.x + 10.0f * (float)sin(y / 7.0f + DirectX::XM_PI * 0.66f * 2.0f) , center.y, center.z + 10.0f * (float)cos(y / 7.0f + DirectX::XM_PI * 0.66f * 2.0f));

			for (UINT x = 0; x < m_width; x++)
			{
				float result = 0;

				//Pilars
				result += 1.0f / (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), pillar1.x, pillar1.y, pillar1.z) / maxDistance) - 1.0f;
				result += 1.0f / (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), pillar2.x, pillar2.y, pillar2.z) / maxDistance) - 1.0f;
				result += 1.0f / (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), pillar3.x, pillar3.y, pillar3.z) / maxDistance) - 1.0f;

				//Water Flow Channel
				result -= 1.0f / (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), center.x, center.y, center.z) / maxDistance) - 1.0f;

				//Teraces
				result += 2.0f * (float)cos(y);

				//Outer Bounds
				result -= pow((getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), center.x, center.y, center.z) / maxDistance), 3.0f);

				//Noise
				float valueX = (float)x / (float)m_width * 3.0f;
				float valueY = (float)y / (float)m_height * 3.0f;
				float valueZ = (float)z / (float)m_depth * 3.0f;

				result += (float)m_perlin.GetValue(valueX + m_noiseOffset, valueY + m_noiseOffset, valueZ + m_noiseOffset) * 5.0f;

				m_data[index] = result;
				index++;
			}
		}

	}
}

bool GeometryData::SetBufferData(ID3D11DeviceContext* context, XMMATRIX world, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, int initialSteps, int refinementSteps, float depthfactor, LightClass& light)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixData;
	LightingBufferType* lightData;
	FactorBufferType* factorData;
	LightMatrixBufferType* lightMatrixData;

	//DirectX11 need matrices transposed!
	XMMATRIX worldM = XMMatrixTranspose(world);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	XMMATRIX lightViewMatrix;
	light.GetViewMatrix(lightViewMatrix);
	lightViewMatrix = XMMatrixTranspose(lightViewMatrix);

	XMMATRIX lightProjectionMatrix;
	light.GetProjectionMatrix(lightProjectionMatrix);
	lightProjectionMatrix = XMMatrixTranspose(lightProjectionMatrix);

	{
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
		matrixData->world = worldM;
		matrixData->view = viewMatrix;
		matrixData->projection = projectionMatrix;

		context->Unmap(matrixBuffer, 0);
	}
	{
		// ### LightingBuffer ###
		//Lock Buffer
		result = context->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		//Get pointer to data
		lightData = static_cast<LightingBufferType*>(mappedResource.pData);

		//Set data
		lightData->cameraPosition = eyePos;
		lightData->lightDirection = light.GetDirection();
		lightData->diffuseColor = light.GetDiffuseColor();
		lightData->ambientColor = light.GetAmbientColor();

		context->Unmap(lightBuffer, 0);
	}
	{
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
	}
	{
		// ### LIGHTMATRIXBUFFER ###
		//Lock Buffer
		result = context->Map(lightMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		if (FAILED(result))
		{
			return false;
		}

		//Get pointer to data
		lightMatrixData = static_cast<LightMatrixBufferType*>(mappedResource.pData);

		//Set data
		lightMatrixData->lightViewMatrix = lightViewMatrix;
		lightMatrixData->lightProjectionMatrix = lightProjectionMatrix;

		context->Unmap(lightMatrixBuffer, 0);
	}

	return true;
}

void GeometryData::LoadTextures(ID3D11Device* device)
{

	TextureClass* rock1 = new TextureClass();
	rock1->Initialize(device, L"./Assets/rock1.dds", L"./Assets/rock1_heightmap.dds");

	TextureClass* rock2 = new TextureClass();
	rock2->Initialize(device, L"./Assets/rock2.dds", L"./Assets/rock2_heightmap.dds");

	TextureClass* rock3 = new TextureClass();
	rock3->Initialize(device, L"./Assets/rock3.dds", L"./Assets/rock3_heightmap.dds");

	TextureClass* brick = new TextureClass();
	brick->Initialize(device, L"./Assets/brickwall.dds", L"./Assets/brickwall_heightmap.dds");

	m_colorTextures[0] = rock1;
	m_colorTextures[1] = rock2;
	m_colorTextures[2] = rock3;
}

void GeometryData::ReadFromGSBuffer(ID3D11DeviceContext* context)
{
	//Reading from Buffer
	GeometryVertexInputType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedRessource;
	ID3D11Buffer* readbuf = marchingCubeGSO->GetReadBuffer(context);

	context->Map(readbuf, 0, D3D11_MAP_READ, 0, &mappedRessource);

	vertices = static_cast<GeometryVertexInputType*>(mappedRessource.pData);

	//Generating Triangles
	for(size_t i = 2u; i < generatedVertexCount; i+=3)
	{
		KdTree::Triangle* tri = new KdTree::Triangle();
		tri->vertices[0] = static_cast<DirectX::XMFLOAT3>(DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vertices[i - 2].position.x, vertices[i - 2].position.y, vertices[i - 2].position.z), worldMatrix));
		tri->vertices[1] = static_cast<DirectX::XMFLOAT3>(DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vertices[i - 1].position.x, vertices[i - 1].position.y, vertices[i - 1].position.z), worldMatrix));
		tri->vertices[2] = static_cast<DirectX::XMFLOAT3>(DirectX::SimpleMath::Vector3::Transform(DirectX::SimpleMath::Vector3(vertices[i].position.x, vertices[i].position.y, vertices[i].position.z), worldMatrix));
		tri->CalculateGreatest();
		tri->CalculateSmallest();
		tree->AddTriangle(tri);
	}

	context->Unmap(readbuf, 0);

	tree->MarkKDTreeDirty();
}

void GeometryData::MarchingCubeRenderpass(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixData;

	//DirectX11 need matrices transposed!
	XMMATRIX world = XMMatrixTranspose(XMMatrixIdentity());
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// ### MATRIXBUFFER ###
	//Lock Buffer
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		printf("Mapping Buffer failed in MarchingCube Renderpass.\n\r");
	}

	//Get pointer to data
	matrixData = static_cast<MatrixBufferType*>(mappedResource.pData);

	//Set data
	matrixData->world = world;
	matrixData->view = viewMatrix;
	matrixData->projection = projectionMatrix;

	deviceContext->Unmap(matrixBuffer, 0);

	UINT offset = 0, stride = sizeof(MarchingCubeVertexInputType);
	deviceContext->SOSetTargets(1, &marchingCubeGSO->outputBuffer, &offset);

	marchingCubeVS->Set(deviceContext);
	marchingCubeGSO->Set(deviceContext);

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//Density Map to use
	deviceContext->GSSetShaderResources(0, 1, &m_densityMap);
	deviceContext->GSSetShaderResources(1, 1, &m_triangleLUT);
	//Set point sampler to use in the geometry shader
	deviceContext->GSSetSamplers(0, 1, &m_densitySampler);
	deviceContext->GSSetConstantBuffers(1, 1, &m_decalDescriptionBuffer);

	deviceContext->GSSetConstantBuffers(0, 1, &matrixBuffer);

	deviceContext->Begin(statsQuery);
	deviceContext->Draw(m_vertexCount, 0);
	deviceContext->End(statsQuery);

	deviceContext->SOSetTargets(0, nullptr, nullptr);
	isGeometryGenerated = true;
	deviceContext->GSSetShader(nullptr, nullptr, 0);

	CountGeneratedTriangles(deviceContext);
	ReadFromGSBuffer(deviceContext);
}

void GeometryData::CountGeneratedTriangles(ID3D11DeviceContext* context)
{
	D3D11_QUERY_DATA_PIPELINE_STATISTICS stats;
	while(S_OK != context->GetData(statsQuery, &stats, sizeof(stats), 0))
	{
		Sleep(1);
	}

	generatedVertexCount = stats.VSInvocations;
	printf("%llu\n\r", generatedVertexCount);
}

ID3D11Buffer* GeometryData::GetGeometryVertexBuffer()
{
	return marchingCubeGSO->outputBuffer;
}

void GeometryData::SetVertexBuffer(ID3D11DeviceContext* context)
{
	UINT stride, offset;
	offset = 0;
	stride = sizeof(GeometryVertexInputType);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetVertexBuffers(0, 1, &marchingCubeGSO->outputBuffer, &stride, &offset);
}

UINT GeometryData::GetGeometryVertexBufferStride()
{
	return sizeof(GeometryVertexInputType);
}

void GeometryData::GenerateNoiseData()
{
	size_t index = 0u;

	for (UINT z = 0; z < m_depth; z++)
	{
		for (UINT y = 0; y < m_height; y++)
		{
			for (UINT x = 0; x < m_width; x++)
			{
				float valueX = (float)x / (float)m_width * 3.0f;
				float valueY = (float)y / (float)m_height * 3.0f;
				float valueZ = (float)z / (float)m_depth * 3.0f;

				m_data[index] = (float)m_perlin.GetValue(valueX + m_noiseOffset, valueY + m_noiseOffset, valueZ + m_noiseOffset);

				index++;
			}
		}

	}
}

void GeometryData::GenerateBumpySphere()
{
	DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(m_width / 2.0f, m_height / 2.0f, m_depth / 2.0f);

	size_t index = 0u;
	float maxDistance = m_width / 2.0f;

	for (UINT z = 0; z < m_depth; z++)
	{
		for (UINT y = 0; y < m_height; y++)
		{
			for (UINT x = 0; x < m_width; x++)
			{
				float result = 1.0f - (getDistance(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), center.x, center.y, center.z) / maxDistance);

				float valueX = (float)x / (float)m_width * 3.0f;
				float valueY = (float)y / (float)m_height * 3.0f;
				float valueZ = (float)z / (float)m_depth * 3.0f;

				result += (float)m_perlin.GetValue(valueX + m_noiseOffset, valueY + m_noiseOffset, valueZ + m_noiseOffset) / 20.0f;

				m_data[index] = result;

				index++;
			}
		}
	}
}

int GeometryData::GetVertices(MarchingCubeVertexInputType** outVertices)
{
	int size = int(2.0f / m_cubeStep.x);
	size = size * size * size;
	m_vertexCount = size;

	(*outVertices) = new MarchingCubeVertexInputType[size];
	int idx = 0;
	for (float z = -1; z < 1.0f; z += m_cubeStep.z)
	{
		for (float y = -1; y < 1.0f; y += m_cubeStep.y)
		{
			for (float x = -1; x < 1.0f; x += m_cubeStep.x)
			{
				(*outVertices)[idx].position = DirectX::XMFLOAT3(x, y, z);
				(*outVertices)[idx].color = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

				idx++;
			}
		}
	}

	return size;
}


bool GeometryData::InitializeBuffers(ID3D11Device* device)
{
	HRESULT result;
	D3D11_BUFFER_DESC matrixBufferDesc, eyeBufferDesc, factorBufferDesc, lightMatrixBufferDesc;
	MarchingCubeVertexInputType* vertices = nullptr;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;

	// Set the number of vertices in the vertex array.
	m_vertexCount = GetVertices(&vertices);

	// VertexBuffer
	{
		
		// Set up the description of the static vertex buffer.
		vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDesc.ByteWidth = sizeof(MarchingCubeVertexInputType) * m_vertexCount;
		vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDesc.CPUAccessFlags = 0;
		vertexBufferDesc.MiscFlags = 0;
		vertexBufferDesc.StructureByteStride = 0;

		// Give the subresource structure a pointer to the vertex data.
		vertexData.pSysMem = vertices;
		vertexData.SysMemPitch = 0;
		vertexData.SysMemSlicePitch = 0;

		// Now create the vertex buffer.
		device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

		// Release the arrays now that the vertex and index buffers have been created and loaded.
		delete[] vertices;
		vertices = nullptr;
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

	//LightingBuffer
	{
		//Setup Lighting Buffer Description
		eyeBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		eyeBufferDesc.ByteWidth = sizeof(LightingBufferType);
		eyeBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		eyeBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		eyeBufferDesc.MiscFlags = 0;
		eyeBufferDesc.StructureByteStride = 0;

		//Make Buffer accessible
		result = device->CreateBuffer(&eyeBufferDesc, nullptr, &lightBuffer);
		if (FAILED(result))
		{
			return false;
		}
	}

	//LightMatrixBuffer
	{
		//Setup matrix Buffer Description
		lightMatrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		lightMatrixBufferDesc.ByteWidth = sizeof(LightMatrixBufferType);
		lightMatrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		lightMatrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		lightMatrixBufferDesc.MiscFlags = 0;
		lightMatrixBufferDesc.StructureByteStride = 0;

		//Make Buffer accessible
		result = device->CreateBuffer(&lightMatrixBufferDesc, nullptr, &lightMatrixBuffer);
		if (FAILED(result))
		{
			return false;
		}
	}

	//PixelDisplacementBuffer
	{
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
	}

	//Query
	{
		D3D11_QUERY_DESC queryDesc;
		queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
		queryDesc.MiscFlags = 0;
		result = device->CreateQuery(&queryDesc, &statsQuery);
		if(FAILED(result))
		{
			printf("Buffer Creation Failed. \n\r");
			return false;
		}
	}

	return true;

}

bool GeometryData::InitializeShaders(ID3D11Device* device)
{
	{
		D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

		//Vertex Input Layout Description
		//needs to mach MarchingCubeVertexInputType
		polygonLayout[0].SemanticName = "SV_POSITION";
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

		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		marchingCubeVS = new VertexShader();
		marchingCubeVS->Initialize(device, L"MarchingCube_VS.hlsl", polygonLayout, numElements);
	}

	{
		D3D11_INPUT_ELEMENT_DESC polygonLayout[4];

		//Vertex Input Layout Description
		//needs to mach GeometryVertexInputType
		polygonLayout[0].SemanticName = "SV_POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		polygonLayout[1].SemanticName = "POSITION";
		polygonLayout[1].SemanticIndex = 0;
		polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[1].InputSlot = 0;
		polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[1].InstanceDataStepRate = 0;

		polygonLayout[2].SemanticName = "COLOR";
		polygonLayout[2].SemanticIndex = 0;
		polygonLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[2].InputSlot = 0;
		polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[2].InstanceDataStepRate = 0;

		polygonLayout[3].SemanticName = "NORMAL";
		polygonLayout[3].SemanticIndex = 0;
		polygonLayout[3].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[3].InputSlot = 0;
		polygonLayout[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		polygonLayout[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[3].InstanceDataStepRate = 0;

		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		geometryVS = new VertexShader();
		geometryVS->Initialize(device, L"Geometry_VS.hlsl", polygonLayout, numElements);
	}

	triplanarDisplacementPS = new PixelShader();
	triplanarDisplacementPS->Initialize(device, L"Triplanar_Displacement_PS.hlsl");


	{
		D3D11_BUFFER_DESC bufferDesc = {};

		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_STREAM_OUTPUT | D3D11_BIND_VERTEX_BUFFER; // Wichtig ist du musst beide Flags setzen, einmal für Output und einmal für vertex buffer.
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;
		bufferDesc.StructureByteStride = 0;
		bufferDesc.ByteWidth = 50 * 1024 * 1024;

		D3D11_SO_DECLARATION_ENTRY declarationEntry[4];

		declarationEntry[0].ComponentCount = 4;
		declarationEntry[0].SemanticIndex = 0;
		declarationEntry[0].SemanticName = "SV_POSITION";
		declarationEntry[0].OutputSlot = 0;
		declarationEntry[0].StartComponent = 0;
		declarationEntry[0].Stream = 0;

		declarationEntry[1].ComponentCount = 4;
		declarationEntry[1].SemanticIndex = 0;
		declarationEntry[1].SemanticName = "POSITION";
		declarationEntry[1].OutputSlot = 0;
		declarationEntry[1].StartComponent = 0;
		declarationEntry[1].Stream = 0;

		declarationEntry[2].ComponentCount = 4;
		declarationEntry[2].SemanticIndex = 0;
		declarationEntry[2].SemanticName = "COLOR";
		declarationEntry[2].OutputSlot = 0;
		declarationEntry[2].StartComponent = 0;
		declarationEntry[2].Stream = 0;

		declarationEntry[3].ComponentCount = 4;
		declarationEntry[3].SemanticIndex = 0;
		declarationEntry[3].SemanticName = "NORMAL";
		declarationEntry[3].OutputSlot = 0;
		declarationEntry[3].StartComponent = 0;
		declarationEntry[3].Stream = 0;

		UINT numElements = sizeof(declarationEntry) / sizeof(declarationEntry[0]);

		marchingCubeGSO = new GeometryOutputShader();
		marchingCubeGSO->Initialize(device, L"MarchingCube_GS.hlsl", bufferDesc, declarationEntry, numElements);
	}

	return true;
}

D3D11_TEXTURE3D_DESC GeometryData::CreateTextureDesc() const
{
	D3D11_TEXTURE3D_DESC output;

	output.Width = m_width;
	output.Height = m_height;
	output.Depth = m_depth;
	output.MipLevels = 1;
	output.Format = DXGI_FORMAT_R32_FLOAT;
	output.Usage = D3D11_USAGE_DEFAULT;
	output.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	output.CPUAccessFlags = 0;
	output.MiscFlags = 0;

	return output;
}

D3D11_SUBRESOURCE_DATA GeometryData::CreateSubresourceData() const
{
	D3D11_SUBRESOURCE_DATA output;

	ZeroMemory(&output, sizeof(output));

	//Size of a 1d line. Eg. distance between adjacent values
	output.SysMemPitch = m_width * sizeof(float);

	//Size of a 2D slice
	output.SysMemSlicePitch = m_width * m_height * sizeof(float);

	//The actual data
	output.pSysMem = m_data;

	return output;
}

ID3D11Texture3D* GeometryData::CreateTexture(ID3D11Device* device, const D3D11_TEXTURE3D_DESC texDesc, const D3D11_SUBRESOURCE_DATA subData) const
{
	ID3D11Texture3D* output;

	device->CreateTexture3D(&texDesc, &subData, &output);

	return output;
}

ID3D11ShaderResourceView* GeometryData::CreateDensityShaderResource(ID3D11Device* device, ID3D11Texture3D* texture3D) const
{
	ID3D11ShaderResourceView* output;

	device->CreateShaderResourceView(texture3D, nullptr, &output);

	return output;
}

ID3D11ShaderResourceView* GeometryData::CreateTriangleLUTShaderResource(ID3D11Device* device) const
{
	ID3D11ShaderResourceView* output;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Height = 256;
	desc.Width = 16;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R32_SINT;
	desc.SampleDesc = { 1, 0 };
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(initData));
	initData.SysMemPitch = 16 * sizeof(int);
	initData.SysMemSlicePitch = 0;

	initData.pSysMem = TriangleLUT::TriTable;

	ID3D11Texture2D* texture = nullptr;
	device->CreateTexture2D(&desc, &initData, &texture);
	device->CreateShaderResourceView(texture, nullptr, &output);

	return output;
}

ID3D11SamplerState* GeometryData::CreateDensitySamplerState(ID3D11Device* device) const
{
	ID3D11SamplerState* output;

	//Create a basic point sampler for sampling our density data in the gpu
	//should refactor this elsewhere
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = 0;

	device->CreateSamplerState(&sampDesc, &output);

	return output;
}

void GeometryData::CreatePSSamplerStates(ID3D11Device* device, ID3D11SamplerState* wrapSampler, ID3D11SamplerState* clampSampler)
{
	//Create a basic point sampler for sampling our density data in the gpu
	//should refactor this elsewhere
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.MipLODBias = 0.0f;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	sampDesc.BorderColor[0] = 0;
	sampDesc.BorderColor[1] = 1;
	sampDesc.BorderColor[2] = 0;
	sampDesc.BorderColor[3] = 1;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&sampDesc, &m_wrapSampler);

	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

	device->CreateSamplerState(&sampDesc, &m_clampSampler);

}

void GeometryData::GenerateDecalDescriptionBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(DecalDescription);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	device->CreateBuffer(&bd, nullptr, &m_decalDescriptionBuffer);
	//return S_OK;

	DecalDescription dbuffer;
	dbuffer = GetDecals();
	deviceContext->UpdateSubresource(m_decalDescriptionBuffer, 0, nullptr, &dbuffer, 0, 0);
}

GeometryData::DecalDescription GeometryData::GetDecals() const
{
	DecalDescription buffer;

	ZeroMemory(&buffer, sizeof(buffer));
	buffer.decal[0] = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1);
	buffer.decal[1] = DirectX::XMFLOAT4(m_cubeStep.x, 0.0f, 0.0f, 1);
	buffer.decal[2] = DirectX::XMFLOAT4(m_cubeStep.x, m_cubeStep.y, 0.0f, 1);
	buffer.decal[3] = DirectX::XMFLOAT4(0.0f, m_cubeStep.y, 0.0f, 1);
	buffer.decal[4] = DirectX::XMFLOAT4(0.0f, 0.0f, m_cubeStep.z, 1);
	buffer.decal[5] = DirectX::XMFLOAT4(m_cubeStep.x, 0.0f, m_cubeStep.z, 1);
	buffer.decal[6] = DirectX::XMFLOAT4(m_cubeStep.x, m_cubeStep.y, m_cubeStep.z, 1);
	buffer.decal[7] = DirectX::XMFLOAT4(0.0f, m_cubeStep.y, m_cubeStep.z, 1);

	buffer.dataStep = DirectX::XMFLOAT4(1.0f / static_cast<float>(m_width), 1.0f / static_cast<float>(m_height), 1.0f / static_cast<float>(m_depth), 1);

	return buffer;
}

void GeometryData::DebugPrint()
{
	char* output = new char[m_width + 1];
	size_t index = 0;

	for (size_t i = 0u; i < m_depth; ++i)
	{
		for (size_t j = 0u; j < m_height; ++j)
		{
			for (size_t k = 0u; k < m_width; ++k)
			{
				if (m_data[index] == -1)
				{
					output[k] = '0';
				}
				else
				{
					output[k] = '1';
				}

				index++;
			}
			output[m_width] = '\0';
			printf("%s\n", output);
		}

		printf("\n===========================\n\n");
	}
}

void GeometryData::Render(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, int initialSteps, int refinementSteps, float depthfactor, LightClass& light, ID3D11ShaderResourceView* shadowMap)
{
	if (!isGeometryGenerated)
	{
		MarchingCubeRenderpass(deviceContext, viewMatrix, projectionMatrix);
	}

	SetBufferData(deviceContext, worldMatrix, viewMatrix, projectionMatrix, eyePos, initialSteps, refinementSteps, depthfactor, light);
	UINT offset = 0, stride = sizeof(GeometryVertexInputType);

	//Set Shaders
	geometryVS->Set(deviceContext);
	triplanarDisplacementPS->Set(deviceContext);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceContext->IASetVertexBuffers(0, 1, &marchingCubeGSO->outputBuffer, &stride, &offset);

	deviceContext->PSSetShaderResources(0, 2, m_colorTextures[0]->GetTextureViewArray());
	deviceContext->PSSetShaderResources(2, 2, m_colorTextures[1]->GetTextureViewArray());
	deviceContext->PSSetShaderResources(4, 2, m_colorTextures[2]->GetTextureViewArray());
	deviceContext->PSSetShaderResources(6, 1, &shadowMap);


	deviceContext->PSSetSamplers(0, 1, &m_wrapSampler);
	deviceContext->PSSetSamplers(1, 1, &m_clampSampler);

	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->VSSetConstantBuffers(1, 1, &lightMatrixBuffer);
	//Set constant buffer
	deviceContext->PSSetConstantBuffers(0, 1, &matrixBuffer);
	deviceContext->PSSetConstantBuffers(2, 1, &lightBuffer);
	deviceContext->PSSetConstantBuffers(3, 1, &factorBuffer);

	//DrawAuto no longer needed as we know the number of vertices generated.
	//deviceContext->DrawAuto();
	deviceContext->Draw(static_cast<UINT>(generatedVertexCount), 0);

	ID3D11ShaderResourceView* pSRV = { nullptr };
	deviceContext->PSSetShaderResources(6, 1, &pSRV);
}

unsigned GeometryData::GetVertexCount()
{
	return m_vertexCount;
}
