#pragma once
#include "VertexShader.h"
#include "GeometryDoubleBufferShader.h"
#include "GeometryShader.h"
#include <directxmath.h>
#include "PixelShader.h"

class ParticleSystem
{
public:
	ParticleSystem(ID3D11Device* device, float x, float y, float z);
	ParticleSystem(const ParticleSystem& other);
	~ParticleSystem();

	void Render(ID3D11DeviceContext* context, float deltaTime, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix);

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
		float age;
	}; 

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	struct ParticleInfosBufferType
	{
		float deltaTime;
		DirectX::XMFLOAT3 padding;
	};

	void InitializeShaders(ID3D11Device* device);
	bool InitializeBuffers(ID3D11Device* device, float x, float y, float z);
	void Kickstart(ID3D11DeviceContext* context);
	void SetBufferData(ID3D11DeviceContext* context, DirectX::XMMATRIX xmmatrix, DirectX::XMMATRIX projection_matrix, float deltaTime);
	void FirstRenderPass(ID3D11DeviceContext* context);
	void SecondRenderPass(ID3D11DeviceContext* context);

	bool isWarmedUp = false;
	const UINT maxParticles = 20000;
	ID3D11Buffer *kickstartVertexBuffer = nullptr;
	ID3D11Buffer* matrixBuffer = nullptr;
	ID3D11Buffer* particleInfosBuffer = nullptr;
	DirectX::XMMATRIX worldMatrix;

};
