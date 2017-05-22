#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <noise\noise.h>
#include <random>
#include <chrono>
#include "TextureClass.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GeometryOutputShader.h"

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
			HELIX
		};
	};

	GeometryData(unsigned int width, unsigned int height, unsigned int depth, TerrainType::Enum type, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~GeometryData();

	void DebugPrint();
	void Render(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp, int initialSteps, int refinementSteps, float depthfactor);
	unsigned int GetVertexCount();
	void MarchingCubeRenderpass(ID3D11DeviceContext* deviceContext, XMMATRIX viewMatrix, XMMATRIX projectionMatrix);
	void CountGeneratedTriangles(ID3D11DeviceContext* context);

	DirectX::XMMATRIX worldMatrix;

	bool isGeometryGenerated = false;
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct EyeBufferType
	{
		XMFLOAT3 position;
		float padding1;
		XMFLOAT3 forward;
		float padding2;
		XMFLOAT3 up;
		float padding3;
	};

	struct FactorBufferType
	{
		int steps_initial;
		int steps_refinement;
		float depth_factor;
		float padding;
	};

	struct GeometryVertexInputType
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 worldPos;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT4 normal;
	};

	struct MarchingCubeVertexInputType
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
	void GenerateHelixStructure();

	bool SetBufferData(ID3D11DeviceContext* context, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp, int initialSteps, int refinementSteps, float depthfactor);
	int GetVertices(MarchingCubeVertexInputType** outVertices);
	bool InitializeBuffers(ID3D11Device* device);
	bool InitializeShaders(ID3D11Device* device);
	D3D11_TEXTURE3D_DESC CreateTextureDesc() const;
	D3D11_SUBRESOURCE_DATA CreateSubresourceData() const;
	ID3D11Texture3D* CreateTexture(ID3D11Device* device, D3D11_TEXTURE3D_DESC texDesc, D3D11_SUBRESOURCE_DATA subData) const;
	ID3D11ShaderResourceView* CreateDensityShaderResource(ID3D11Device* device, ID3D11Texture3D* texture3D) const;
	ID3D11ShaderResourceView* CreateTriangleLUTShaderResource(ID3D11Device* device) const;
	ID3D11SamplerState* CreateDensitySamplerState(ID3D11Device* device) const;
	void CreatePSSamplerStates(ID3D11Device* device, ID3D11SamplerState* wrapSampler, ID3D11SamplerState* clampSampler);
	void GenerateDecalDescriptionBuffer(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	DecalDescription GetDecals() const;
	void LoadTextures(ID3D11Device* device);
	void ReadFromGSBuffer(ID3D11DeviceContext* context);

	D3D11_TEXTURE3D_DESC m_texDesc;
	D3D11_SUBRESOURCE_DATA m_subData;
	ID3D11Texture3D* m_texture3D;
	ID3D11ShaderResourceView* m_densityMap;
	ID3D11ShaderResourceView* m_triangleLUT;
	ID3D11SamplerState *m_densitySampler, *m_wrapSampler, *m_clampSampler;
	ID3D11Buffer *m_vertexBuffer = nullptr;
	ID3D11Buffer* m_decalDescriptionBuffer = nullptr;
	ID3D11Buffer* matrixBuffer, *eyeBuffer, *factorBuffer;
	ID3D11Query* statsQuery;

	VertexShader* marchingCubeVS, *geometryVS;
	PixelShader* triplanarDisplacementPS;
	GeometryOutputShader* marchingCubeGSO;

	//Textures
	TextureClass* m_colorTextures[3] = {nullptr};

	float* m_data;
	unsigned int m_width, m_height, m_depth;
	unsigned int m_vertexCount;
	DirectX::XMFLOAT3 m_cubeSize;
	DirectX::XMFLOAT3 m_cubeStep;
	noise::module::Perlin m_perlin;
	double m_noiseOffset;
	UINT64 generatedVertexCount = 0;
	GeometryVertexInputType* generatedVertices;
};
