#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <noise\noise.h>
#include <random>
#include <chrono>

class GeometryData
{
public:

	struct TerrainType
	{
		enum Enum
		{
			CUBE,
			SPHERE,
			BUMPY_SPHERE,
			PILLAR,
			NOISE,
			BUMPY_PILLAR
		};
	};

	GeometryData(unsigned int width, unsigned int height, unsigned int depth, TerrainType::Enum type, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~GeometryData();

	void DebugPrint();
	void Render(ID3D11DeviceContext* deviceContext);
	unsigned int GetVertexCount();

	DirectX::XMMATRIX worldMatrix;
private:

	struct VertexInputType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};

	struct DecalDescription
	{
		DirectX::XMFLOAT4 decal[8];
		DirectX::XMFLOAT4 dataStep;
	};

	void GenerateCubeData();
	float getDistance(const float& p1x, const float& p1y, const float& p1z, const float& p2x, const float& p2y, const float& p2z);
	float getDistance2D(const float& p1x, const float& p1y, const float& p2x, const float& p2y);
	void GenerateSphereData();
	void GeneratePillarData();
	void GenerateNoiseData();
	void GenerateBumpySphere();
	void GenerateBumpyPillar();

	int GetVertices(VertexInputType** outVertices);
	void InitializeBuffers(ID3D11Device*);
	D3D11_TEXTURE3D_DESC CreateTextureDesc() const;
	D3D11_SUBRESOURCE_DATA CreateSubresourceData() const;
	ID3D11Texture3D* CreateTexture(ID3D11Device* device, D3D11_TEXTURE3D_DESC texDesc, D3D11_SUBRESOURCE_DATA subData) const;
	ID3D11ShaderResourceView* CreateDensityShaderResource(ID3D11Device* device, ID3D11Texture3D* texture3D) const;
	ID3D11ShaderResourceView* CreateTriangleLUTShaderResource(ID3D11Device* device) const;
	ID3D11SamplerState* CreateSamplerState(ID3D11Device* device) const;
	void GenerateDecalDescriptionBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	DecalDescription GetDecals() const;

	D3D11_TEXTURE3D_DESC m_texDesc;
	D3D11_SUBRESOURCE_DATA m_subData;
	ID3D11Texture3D* m_texture3D;
	ID3D11ShaderResourceView* m_densityMap;
	ID3D11ShaderResourceView* m_triangleLUT;
	ID3D11SamplerState* m_sampler;
	ID3D11Buffer *m_vertexBuffer = nullptr;;
	ID3D11Buffer* m_decalDescriptionBuffer = nullptr;

	float* m_data;
	unsigned int m_width, m_height, m_depth;
	unsigned int m_vertexCount;
	DirectX::XMFLOAT3 m_cubeSize;
	DirectX::XMFLOAT3 m_cubeStep;
	noise::module::Perlin m_perlin;
	double m_noiseOffset;
};
