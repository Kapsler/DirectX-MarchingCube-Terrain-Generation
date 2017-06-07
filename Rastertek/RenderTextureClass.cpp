#include "RenderTextureClass.h"

RenderTextureClass::RenderTextureClass()
{
	renderTargetTexture = nullptr;
	renderTargetView = nullptr;
	shaderResourceView = nullptr;
}

RenderTextureClass::RenderTextureClass(const RenderTextureClass&)
{
}

RenderTextureClass::~RenderTextureClass()
{
}

bool RenderTextureClass::Initialize(ID3D11Device* device, int textureWidth, int textureHeight, float screenDepth, float screenNear, int sampleCount, int qualityLevel)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;


	ZeroMemory(&textureDesc, sizeof(textureDesc));


	//Setup Render target texture desc
	textureDesc.Width = textureWidth;
	textureDesc.Height = textureHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;


	//Multisample
	textureDesc.SampleDesc.Count = sampleCount;
	textureDesc.SampleDesc.Quality = qualityLevel;

	// Create the render target texture.
	result = device->CreateTexture2D(&textureDesc, nullptr, &renderTargetTexture);
	if (FAILED(result))
	{
		return false;
	}

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = device->CreateRenderTargetView(renderTargetTexture, &renderTargetViewDesc, &renderTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	result = device->CreateShaderResourceView(renderTargetTexture, &shaderResourceViewDesc, &shaderResourceView);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = textureWidth;
	depthBufferDesc.Height = textureHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	//Multisample
	depthBufferDesc.SampleDesc.Count = sampleCount;
	depthBufferDesc.SampleDesc.Quality = qualityLevel;

	// Create the texture for the depth buffer using the filled out description.
	result = device->CreateTexture2D(&depthBufferDesc, nullptr, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Setup the viewport for rendering.
	m_viewport.Width = static_cast<float>(textureWidth);
	m_viewport.Height = static_cast<float>(textureHeight);
	m_viewport.MinDepth = 0.0f;
	m_viewport.MaxDepth = 1.0f;
	m_viewport.TopLeftX = 0.0f;
	m_viewport.TopLeftY = 0.0f;

	// Setup the projection matrix.
	projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, static_cast<float>(static_cast<float>(textureWidth) / static_cast<float>(textureHeight)), screenNear, screenDepth);

	// Create an orthographic projection matrix for 2D rendering.
	orthoMatrix = XMMatrixOrthographicLH(static_cast<float>(textureWidth), static_cast<float>(textureHeight), screenNear, screenDepth);
	
	return true;

}

void RenderTextureClass::Shutdown()
{
	if(m_depthStencilView)
	{
		m_depthStencilView->Release();
		m_depthStencilView = nullptr;
	}

	if(m_depthStencilBuffer)
	{
		m_depthStencilBuffer->Release();
		m_depthStencilBuffer = nullptr;
	}

	if(shaderResourceView)
	{
		shaderResourceView->Release();
		shaderResourceView = nullptr;
	}

	if(renderTargetView)
	{
		renderTargetView->Release();
		renderTargetView = nullptr;
	}

	if(renderTargetTexture)
	{
		renderTargetTexture->Release();
		renderTargetTexture = nullptr;
	}


}

void RenderTextureClass::SetRenderTarget(ID3D11DeviceContext* context)
{
	context->OMSetRenderTargets(1, &renderTargetView, m_depthStencilView);
	context->RSSetViewports(1, &m_viewport);
}

void RenderTextureClass::ClearRenderTarget(ID3D11DeviceContext* context, float red, float green, float blue, float alpha)
{
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	context->ClearRenderTargetView(renderTargetView, color);

	context->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

ID3D11ShaderResourceView* RenderTextureClass::GetShaderResourceView()
{
	return shaderResourceView;
}

void RenderTextureClass::GetProjectionMatrix(XMMATRIX& otherproj)
{
	otherproj = projectionMatrix;
}

void RenderTextureClass::GetOrthoMatrix(XMMATRIX& otherortho)
{
	otherortho = orthoMatrix;
}
