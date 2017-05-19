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

	void Render(ID3D11DeviceContext* context);

	VertexShader* vs = nullptr;
	GeometryDoubleBufferShader* particleUpdateGS = nullptr;
	GeometryShader* particleVisualsGS = nullptr;
	PixelShader* ps = nullptr;


private:

	struct ParticleAttributes
	{
		DirectX::XMFLOAT4 position;
		UINT type;
	};

	void InitializeShaders(ID3D11Device* device);
	void InitializeBuffers(ID3D11Device* device);
	void Kickstart(ID3D11DeviceContext* context);

	bool isWarmedUp = false;
	ID3D11Buffer *kickstartVertexBuffer = nullptr;

};
