#pragma once
#include <d3d11.h>
#include <Windows.h>
#include <directxmath.h>

using namespace DirectX;

class RenderTextureClass
{
public:
	RenderTextureClass();
	RenderTextureClass(const RenderTextureClass&);
	~RenderTextureClass();

	bool Initialize(ID3D11Device*, int, int, float, float, int sampleCount, int qualityLevel);
	void Shutdown();

	void SetRenderTarget(ID3D11DeviceContext*);
	void ClearRenderTarget(ID3D11DeviceContext*, float, float, float, float);
	ID3D11ShaderResourceView* GetShaderResourceView();
	void GetProjectionMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

private:
	ID3D11Texture2D* renderTargetTexture;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11ShaderResourceView* shaderResourceView;
	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilView* m_depthStencilView;
	D3D11_VIEWPORT m_viewport;
	XMMATRIX projectionMatrix, orthoMatrix;
};
