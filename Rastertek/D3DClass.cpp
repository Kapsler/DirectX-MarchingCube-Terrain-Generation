#include "D3DClass.h"
#include <iostream>

D3DClass::D3DClass()
{
	swapChain = nullptr;
	device = nullptr;
	deviceContext = nullptr;
	renderTargetView = nullptr;
	depthStencilBuffer = nullptr;
	depthStencilState = nullptr;
	depthStencilView = nullptr;
	rasterizerState = nullptr;
}

D3DClass::D3DClass(const D3DClass&)
{
}

D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(int screenWidth, int screenHeight, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, numerator = 0, denominator = 1;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDescription;
	DXGI_SWAP_CHAIN_DESC swapChainDescription;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDescription;
	D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDescription;
	D3D11_RASTERIZER_DESC rasterizerDescription;
	float fieldOfView, screenAspect;

	vsync_enabled = vsync;

	//interface factory
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&factory));
	if(FAILED(result))
	{
		return false;
	}

	//video card adapter
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result))
	{
		return false;
	}

	//enumerate monitor
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result))
	{
		return false;
	}

	//Get monitor modes
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, nullptr);
	if (FAILED(result))
	{
		return false;
	}

	//create display mode list
	displayModeList = new DXGI_MODE_DESC[numModes];
	if(!displayModeList)
	{
		return false;
	}

	//fill display mode list
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	//Find right mode in list
	for(unsigned int i = 0; i<numModes; i++)
	{
		if(displayModeList[i].Width == screenWidth && displayModeList[i].Height == screenHeight)
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}

	//Get video card description
	result = adapter->GetDesc(&adapterDescription);
	if (FAILED(result))
	{
		return false;
	}

	//video card memory in MB
	videoCardMemory = static_cast<int>(adapterDescription.DedicatedVideoMemory / 1024 / 1024);

	//convert Name of video card to character array and store it
	int error = wcstombs_s(&stringLength, videoCardDescription, 128, adapterDescription.Description, 128);
	if(error != 0)
	{
		return false;
	}

	//Release all this shit
	delete[] displayModeList;
	displayModeList = nullptr;

	adapterOutput->Release();
	adapterOutput = nullptr;

	adapter->Release();
	adapter = nullptr;

	factory->Release();
	factory = nullptr;

	//Set feature level
	featureLevel = D3D_FEATURE_LEVEL_11_0;

	result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION, &device, nullptr, &deviceContext);
	if (FAILED(result))
	{
		return false;
	}

	//Swap Chain (Front and Back buffer)
	ZeroMemory(&swapChainDescription, sizeof(swapChainDescription));
	
	//Set to 1 backbuffer
	swapChainDescription.BufferCount = 1;

	swapChainDescription.BufferDesc.Width = screenWidth;
	swapChainDescription.BufferDesc.Height = screenHeight;

	swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//Set refresh rate
	if(vsync_enabled)
	{
		swapChainDescription.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = denominator;
	} else {
		swapChainDescription.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	swapChainDescription.OutputWindow = hwnd;

	//Multisample
	FindMultiSampleModes();

	swapChainDescription.SampleDesc.Count = sampleCountModes[currentSampleIndex];
	swapChainDescription.SampleDesc.Quality = currentQuality;


	//Fullscreen or window
	if (fullscreen)
	{
		swapChainDescription.Windowed = false;
	} else
	{
		swapChainDescription.Windowed = true;
	}

	//Scanline ordering
	swapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	//Discard backbuffer after swap
	swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//advanced flags off
	swapChainDescription.Flags = 0;


	//Create swap chain, direct3d device and direct3d device context
	//Will fail if hardware not dx11 ready
	//Can fail with multiple video cards -> if happens iterate cards and choose right one
	result = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_DEBUG, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDescription, &swapChain, &device, nullptr, &deviceContext);
	if(FAILED(result))
	{
		return false;
	}

	//get back buffer pointer
	result = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBufferPtr));
	if(FAILED(result))
	{
		return false;
	}

	//Create renderTargetView with back buffer
	result = device->CreateRenderTargetView(backBufferPtr, nullptr, &renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	//release pointer to back buffer
	backBufferPtr->Release();
	backBufferPtr = nullptr;

	//Initialze depth buffer desc
	ZeroMemory(&depthBufferDescription, sizeof(depthBufferDescription));

	//Set up description
	depthBufferDescription.Width = screenWidth;
	depthBufferDescription.Height = screenHeight;
	depthBufferDescription.MipLevels = 1;
	depthBufferDescription.ArraySize = 1;
	depthBufferDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDescription.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDescription.CPUAccessFlags = 0;
	depthBufferDescription.MiscFlags = 0;

	//Multisample
	depthBufferDescription.SampleDesc.Count = sampleCountModes[currentSampleIndex];
	depthBufferDescription.SampleDesc.Quality = currentQuality;

	//Texture for depth buffer
	result = device->CreateTexture2D(&depthBufferDescription, nullptr, &depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	//Initialize depth stencil desc
	ZeroMemory(&depthStencilDescription, sizeof(depthStencilDescription));
	
	//Set up desc
	depthStencilDescription.DepthEnable = true;
	depthStencilDescription.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDescription.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDescription.StencilEnable = true;
	depthStencilDescription.StencilReadMask = 0xFF;
	depthStencilDescription.StencilWriteMask = 0xFF;

	//Stencil operations if pixel is front facing
	depthStencilDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	//if pixel is back facing
	depthStencilDescription.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDescription.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDescription.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	
	//create depth stencil state
	result = device->CreateDepthStencilState(&depthStencilDescription, &depthStencilState);
	if(FAILED(result))
	{
		return false;
	}

	//Set depth stencil state
	deviceContext->OMSetDepthStencilState(depthStencilState, 1);

	//init depth stencil view
	ZeroMemory(&depthStencilViewDescription, sizeof(depthStencilViewDescription));

	//Set up depth stencil view desc
	depthStencilViewDescription.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDescription.Texture2D.MipSlice = 0;

	result = device->CreateDepthStencilView(depthStencilBuffer, &depthStencilViewDescription, &depthStencilView);
	if(FAILED(result))
	{
		return false;
	}

	//BInd render target view and depth stencil buffer to output render pipeline
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	//Set up raster description, determines how polygons are drawn
	rasterizerDescription.AntialiasedLineEnable = true;
	rasterizerDescription.CullMode = D3D11_CULL_NONE;
	rasterizerDescription.DepthBias = 0;
	rasterizerDescription.DepthBiasClamp = 0.0f;
	rasterizerDescription.DepthClipEnable = true;
	rasterizerDescription.FillMode = D3D11_FILL_WIREFRAME;
	rasterizerDescription.FrontCounterClockwise = false;
	rasterizerDescription.MultisampleEnable = true;
	rasterizerDescription.ScissorEnable = false;
	rasterizerDescription.SlopeScaledDepthBias = 0.0f;

	//create rasterizer state
	result = device->CreateRasterizerState(&rasterizerDescription, &rasterizerState);
	if(FAILED(result))
	{
		return false;
	}

	//Set rasterizer state
	deviceContext->RSSetState(rasterizerState);

	//setup viewport
	viewport.Width = static_cast<float>(screenWidth);
	viewport.Height = static_cast<float>(screenHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	//create viewport
	deviceContext->RSSetViewports(1, &viewport);

	//Set up projection matrix
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = static_cast<float>(screenWidth) / static_cast<float>(screenHeight);
	
	//create projection matrix
	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	//Init world matrix to identity matrix
	worldMatrix = XMMatrixIdentity();

	//Create ortho projection matrix for 2D rendering
	orthoMatrix = XMMatrixOrthographicLH(static_cast<float>(screenWidth), static_cast<float>(screenHeight), screenNear, screenDepth);

	return true;
}

void D3DClass::Shutdown()
{
	//Before shutting down we change to window mode
	//avoids exceptions
	if(swapChain)
	{
		swapChain->SetFullscreenState(false, nullptr);
	}

	if(rasterizerState)
	{
		rasterizerState->Release();
		rasterizerState = nullptr;
	}

	if (depthStencilView)
	{
		depthStencilView->Release();
		depthStencilView = nullptr;
	}

	if (depthStencilState)
	{
		depthStencilState->Release();
		depthStencilState = nullptr;
	}

	if (depthStencilBuffer)
	{
		depthStencilBuffer->Release();
		depthStencilBuffer = nullptr;
	}

	if (renderTargetView)
	{
		renderTargetView->Release();
		renderTargetView = nullptr;
	}

	if (deviceContext)
	{
		deviceContext->Release();
		deviceContext = nullptr;
	}

	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (swapChain)
	{
		swapChain->Release();
		swapChain = nullptr;
	}
}

void D3DClass::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	deviceContext->ClearRenderTargetView(renderTargetView, color);

	deviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void D3DClass::EndScene()
{
	if(vsync_enabled)
	{
		//lock refresh rate
		swapChain->Present(1, 0);
	} else
	{
		//dont wait
		swapChain->Present(0, 0);
	}
}

ID3D11Device* D3DClass::GetDevice()
{
	return device;
}

ID3D11DeviceContext* D3DClass::GetDeviceContext()
{
	return deviceContext;
}

void D3DClass::GetProjectionMatrix(XMMATRIX& otherProj)
{
	otherProj = projectionMatrix;
}

void D3DClass::GetWorldMatrix(XMMATRIX& otherWorld)
{
	otherWorld = worldMatrix;
}

void D3DClass::GetOrthoMatrix(XMMATRIX& otherOrtho)
{
	otherOrtho = orthoMatrix;
}

void D3DClass::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, videoCardDescription);
	memory = videoCardMemory;
}

void D3DClass::SetBackBufferRenderTarget()
{
	deviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
}

void D3DClass::ResetViewport()
{
	deviceContext->RSSetViewports(1, &viewport);
}

void D3DClass::ChangeMultiSampleMode(int newsamplecount, int newqualitylevel)
{
	std::cout << newsamplecount << newqualitylevel << std::endl;
}

void D3DClass::FindMultiSampleModes()
{	
	//Multisample

	int sampleCount;
	UINT qualityLevels;

	for(int i = 0; i <= D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT; i++)
	{
		sampleCount = i;

		device->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, sampleCount, &qualityLevels);

		if (qualityLevels > 0)
		{
			sampleCountModes.push_back(i);
			maxQualityLevels.insert(std::pair<int, UINT>(sampleCount, qualityLevels-1));
		}
	}
	
	currentSampleIndex = 0;
	currentQuality = 0;
}

int D3DClass::GetCurrentSampleCount()
{
	return sampleCountModes[currentSampleIndex];
}

int D3DClass::GetCurrentQualityLevel()
{
	return currentQuality;
}

void D3DClass::IncreaseSampleCount()
{
	currentSampleIndex += 1;
	if (currentSampleIndex >= sampleCountModes.size())
	{
		currentSampleIndex = 0;
	}
	if (currentQuality > maxQualityLevels.find(sampleCountModes[currentSampleIndex])->second)
	{
		currentQuality = 0;
	}
}

void D3DClass::IncreaseQualityLevel()
{
	currentQuality += 1;
	if (currentQuality > maxQualityLevels.find(sampleCountModes[currentSampleIndex])->second)
	{
		currentQuality = 0;
	}
}

int D3DClass::GetMaxSampleCount()
{
	return *(sampleCountModes.end() - 1);
}

int D3DClass::GetMaxQualityLevels()
{
	return maxQualityLevels.find(sampleCountModes[currentSampleIndex])->second;
}
