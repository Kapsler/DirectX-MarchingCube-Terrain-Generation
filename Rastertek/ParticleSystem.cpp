#include "ParticleSystem.h"

ParticleSystem::ParticleSystem(ID3D11Device* device)
{
	InitializeShaders(device);
}

ParticleSystem::ParticleSystem(const ParticleSystem& other)
{

}

ParticleSystem::~ParticleSystem()
{

}

void ParticleSystem::Render(ID3D11DeviceContext* context)
{

}

void ParticleSystem::InitializeShaders(ID3D11Device* device)
{
	{
		D3D11_INPUT_ELEMENT_DESC polygonLayout[1];

		//Vertex Input Layout Description
		polygonLayout[0].SemanticName = "SV_POSITION";
		polygonLayout[0].SemanticIndex = 0;
		polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		polygonLayout[0].InputSlot = 0;
		polygonLayout[0].AlignedByteOffset = 0;
		polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		polygonLayout[0].InstanceDataStepRate = 0;

		UINT numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

		vs = new VertexShader();
		vs->Initialize(device, L"Particle_VS.hlsl", polygonLayout, numElements);
	}

	{
		
	}
}

void ParticleSystem::InitializeBuffers(ID3D11Device* device)
{

}
