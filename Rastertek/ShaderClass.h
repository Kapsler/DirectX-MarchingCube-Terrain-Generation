#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <fstream>
#include "VertexShader.h"
#include "PixelShader.h"
#include "GeometryShader.h"
using namespace DirectX;
using namespace std;

class ShaderClass
{
public:
	ShaderClass();
	ShaderClass(const ShaderClass&);
	~ShaderClass();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext* context, int indexCount, int instanceCount,
		XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX	projectionMatrix,
		XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp,
		int intitialSteps, int refinementSteps, float depthfactor);
	bool Render(ID3D11DeviceContext* context, int vertexCount,
		XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX	projectionMatrix
		, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp, 
		int intitialSteps, int refinementSteps, float depthfactor);

private:
	bool InitializeShader(ID3D11Device*, HWND, WCHAR*, WCHAR* geometryFilename, WCHAR*);
	void ShutdownShader();

	bool SetShaderParameters(ID3D11DeviceContext* context, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 eyePos, XMFLOAT3 eyeDir, XMFLOAT3 eyeUp, int intitialSteps, int refinementSteps, float depthfactor);
	void RenderShader(ID3D11DeviceContext*, int vertexCount);
	void RenderShader(ID3D11DeviceContext*, int indexCount, int instanceCount);

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

	ID3D11Buffer* matrixBuffer, *eyeBuffer, *factorBuffer;
	ID3D11SamplerState* sampleStateWrap;
	ID3D11SamplerState* sampleStateClamp;

	VertexShader* vs;
	PixelShader* ps;
	GeometryShader* gs;
};