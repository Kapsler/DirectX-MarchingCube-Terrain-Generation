#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "DirectXTK.lib")

#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <unordered_map>
using namespace DirectX;

class D3DClass
{
public:
	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(XMMATRIX&);
	void GetWorldMatrix(XMMATRIX&);
	void GetOrthoMatrix(XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	void SetBackBufferRenderTarget();
	void ResetViewport();
	void ChangeMultiSampleMode(int newsamplecount, int newqualitylevel);
	void FindMultiSampleModes();
	int GetCurrentSampleCount();
	int GetCurrentQualityLevel();
	void IncreaseSampleCount();
	void IncreaseQualityLevel();
	int GetMaxSampleCount();
	int GetMaxQualityLevels();

	D3D11_VIEWPORT viewport;
	IDXGISwapChain* swapChain;
private:
	bool vsync_enabled;
	int videoCardMemory;
	char videoCardDescription[128];
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11DepthStencilState* depthStencilState;
	ID3D11DepthStencilView* depthStencilView;
	ID3D11RasterizerState* rasterizerState;
	XMMATRIX projectionMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX orthoMatrix;

	std::vector<int> sampleCountModes;
	std::unordered_multimap<int, UINT> maxQualityLevels;
	unsigned int currentSampleIndex;
	unsigned int currentQuality;

};