#pragma once
#include <d3d11.h>
#include <directxmath.h>


class GeometryData
{
public:

	struct TerrainType
	{
		enum Enum
		{
			CUBE
		};
	};

	GeometryData(unsigned int width, unsigned int height, unsigned int depth, TerrainType::Enum type, ID3D11Device* device);
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

	void GenerateCubeData();

	int GetVertices(VertexInputType** outVertices);
	void InitializeBuffers(ID3D11Device*);
	D3D11_TEXTURE3D_DESC CreateTextureDesc() const;
	D3D11_SUBRESOURCE_DATA CreateSubresourceData() const;
	ID3D11Texture3D* CreateTexture(ID3D11Device* device, D3D11_TEXTURE3D_DESC texDesc, D3D11_SUBRESOURCE_DATA subData) const;
	ID3D11ShaderResourceView* CreateShaderResourceView(ID3D11Device* device, ID3D11Texture3D* texture3D) const;

	D3D11_TEXTURE3D_DESC m_texDesc;
	D3D11_SUBRESOURCE_DATA m_subData;
	ID3D11Texture3D* m_texture3D;
	ID3D11ShaderResourceView* m_shaderResourceView;
	ID3D11Buffer *m_vertexBuffer;

	float* m_data;
	unsigned int m_width, m_height, m_depth;
	unsigned int m_vertexCount;
	DirectX::XMFLOAT3 m_cubeSize;
	DirectX::XMFLOAT3 m_cubeStep;
};
