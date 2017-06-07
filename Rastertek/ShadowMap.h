#pragma once
#include "VertexShader.h"
#include "PixelShader.h"
#include <directxmath.h>
#include "RenderTextureClass.h"
#include "LightClass.h"

class ShadowMap
{
	
public:
	ShadowMap(ID3D11Device* device, int sampleCount, int qualityLevel);
	ShadowMap(const ShadowMap& other);
	~ShadowMap();

	void Prepare(ID3D11DeviceContext* deviceContext);
	void Render(ID3D11DeviceContext* deviceContext, const UINT& vertexCount, const XMMATRIX& worldMatrix, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix);
	ID3D11ShaderResourceView* GetShaderResourceView();

private:

	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	struct ShadowVertexInputType
	{
		DirectX::XMFLOAT4 position;
	};

	bool InitializeBuffers(ID3D11Device* device, int sampleCount, int qualityLevel);
	bool InitializeShaders(ID3D11Device* device);	
	bool SetBufferData(ID3D11DeviceContext* context, const XMMATRIX& worldMatrix, const XMMATRIX& lightViewMatrix, const XMMATRIX& lightProjectionMatrix);

	VertexShader* vs;
	PixelShader* ps;
	RenderTextureClass*	shadowMapTexture;
	XMMATRIX lightViewMatrix, lightProjectionMatrix;
	ID3D11Buffer* matrixBuffer;
};
