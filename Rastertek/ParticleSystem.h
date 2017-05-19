#pragma once
#include "VertexShader.h"
#include "GeometryDoubleBufferShader.h"

class ParticleSystem
{
public:
	ParticleSystem(ID3D11Device* device);
	ParticleSystem(const ParticleSystem& other);
	~ParticleSystem();

	void Render(ID3D11DeviceContext* context);

	VertexShader* vs = nullptr;
	GeometryDoubleBufferShader* gdbs = nullptr;


private:
	
	void InitializeShaders(ID3D11Device* device);
	void InitializeBuffers(ID3D11Device* device);

};
