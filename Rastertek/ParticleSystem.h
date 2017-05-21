#pragma once
#include "VertexShader.h"
#include "GeometryDoubleBufferShader.h"
#include "GeometryShader.h"
#include <directxmath.h>
#include "PixelShader.h"

class ParticleSystem
{
public:
	ParticleSystem(ID3D11Device* device);
	ParticleSystem(const ParticleSystem& other);
	~ParticleSystem();

	void Render(ID3D11DeviceContext* context, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);

	VertexShader* particleUpdateVS = nullptr;
	GeometryDoubleBufferShader* particleUpdateGS = nullptr;
	VertexShader* particleVisualsVS = nullptr;
	GeometryShader* particleVisualsGS = nullptr;
	PixelShader* ps = nullptr;


private:

	struct ParticleAttributes
	{
		DirectX::XMFLOAT4 position;
		UINT type;
	}; 
	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};


	void InitializeShaders(ID3D11Device* device);
	bool InitializeBuffers(ID3D11Device* device);
	void Kickstart(ID3D11DeviceContext* context);
	void SetBufferData(ID3D11DeviceContext* context, DirectX::XMMATRIX xmmatrix, DirectX::XMMATRIX projection_matrix);
	void FirstRenderPass(ID3D11DeviceContext* context);
	void SecondRenderPass(ID3D11DeviceContext* context);

	bool isWarmedUp = false;
	ID3D11Buffer *kickstartVertexBuffer = nullptr; 
	ID3D11Buffer* matrixBuffer = nullptr;
	DirectX::XMMATRIX worldMatrix;

};
