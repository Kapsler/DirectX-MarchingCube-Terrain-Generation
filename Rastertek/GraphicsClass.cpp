#include "GraphicsClass.h"
#include <memory>
#include <SpriteFont.h>
#include <SimpleMath.inl>
#include <string>
#include "modelclass.h"

GraphicsClass::GraphicsClass()
{
	direct3D = nullptr;
	camera = nullptr;
	light = nullptr;
	renderTexture = nullptr;
	timer = nullptr;
	depthShader = nullptr;
	bumpiness = 1;
	screenBuffer = nullptr;
	screenTargetView = nullptr;
	depthBuffer = nullptr;
	depthTargetView = nullptr;
}

GraphicsClass::GraphicsClass(const GraphicsClass&)
{
	//TODO Most Copy Constructors in this project still need to be implemented
}

GraphicsClass::~GraphicsClass()
{
}

bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;
	
	currentScreenWidth = screenWidth;
	currentScreenHeight = screenHeight;
	hwndptr = &hwnd;

	//Set up Direct3D
	direct3D = new D3DClass();
	if (!direct3D)
	{
		return false;
	}

	result = direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	//Set up Camera
	camera = new CameraClass();
	if (!camera)
	{
		return false;
	}

	result = camera->Initialize(direct3D->GetDevice());
	if (!result)
	{
		return false;
	}

	camera->SetPosition(0.0f, 0.0f, -5.0f);


	timer = new TimerClass();
	if (!timer)
	{
		return false;
	}

	result = timer->Initialize();
	if (!result)
	{
		return false;
	}

	//HARDCODED

	//Lights
	light = new LightClass();
	if (!light)
	{
		return false;
	}
	light->SetPosition(30.0f, 100.0f, 100.0f);
	light->SetAmbientColor(0.2f, 0.2f, 0.2f, 1.0f);
	light->SetDiffuseColor(1.0, 1.0, 1.0, 1.0f);
	light->SetLookAt(0.0f, 0.0f, 0.0f);
	light->GenerateProjectionsMatrix(SCREEN_DEPTH, SCREEN_NEAR);

	RegenrateTerrain();
	//SpawnParticles(0, 0, 0);

	//HARDCODED END

	//Create RenderToTexture
	renderTexture = new RenderTextureClass();
	if (!renderTexture)
	{
		return false;
	}

	result = renderTexture->Initialize(direct3D->GetDevice(), SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, SCREEN_DEPTH, SCREEN_NEAR, direct3D->GetCurrentSampleCount(), direct3D->GetCurrentQualityLevel());
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the render to texture object.", L"Error", MB_OK);
		return false;
	}

	depthShader = new DepthShaderClass();
	if (!depthShader)
	{
		return false;
	}

	result = depthShader->Initialize(direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the depth shader object.", L"Error", MB_OK);
		return false;
	}

	//Text
	m_font.reset(new SpriteFont(direct3D->GetDevice(), L"./Assets/myfile.spritefont"));

	m_spriteBatch.reset(new SpriteBatch(direct3D->GetDeviceContext()));

	fpsPos.x = screenWidth / 80.0f;
	fpsPos.y = screenHeight / 80.0f + 10;
	scPos.x = screenWidth / 80.0f;
	scPos.y = screenHeight / 80.0f + 50;
	qlPos.x = screenWidth / 80.0f;
	qlPos.y = screenHeight / 80.0f + 90;

	GenerateScreenBuffer();

	SetupPrimitiveBatch();

	return true;
}

void GraphicsClass::Shutdown()
{
	if (depthShader)
	{
		depthShader->Shutdown();
		delete depthShader;
		depthShader = nullptr;
	}

	if (renderTexture)
	{
		renderTexture->Shutdown();
		delete renderTexture;
		renderTexture = nullptr;
	}

	if(terrain)
	{
		delete terrain;
		terrain = nullptr;
	}


	if (light)
	{
		delete light;
		light = nullptr;
	}

	if (timer)
	{
		delete timer;
		timer = nullptr;
	}

	if (camera)
	{
		camera->Shutdown();
		delete camera;
		camera = nullptr;
	}

	if (direct3D)
	{
		direct3D->Shutdown();
		delete direct3D;
		direct3D = nullptr;
	}
}

bool GraphicsClass::Frame(InputClass* input)
{
	bool result;
	static float counter = -5.0f;
	static float totaltime = 0.0f;
	timer->Frame();
	static float rotation = 0.0f;

	float deltaTime = timer->GetFrameTime();
	totaltime += deltaTime;

	if(totaltime > 1000)
	{
		fps = 1000.0f / deltaTime;
		totaltime = 0.0f;
	}

	CheckWireframe(input); 
	CheckMSKeys(input);
	CheckTerrainKey(input);
	CheckRotationKey(input); 
	CheckFactorKeys(input);
	CheckRaycast(input);

	//tree.UpdateKDTree();

	// Update the rotation variable each frame.
	rotation += static_cast<float>(XM_PI) * 0.0001f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}


	result = Render(rotation, input, deltaTime);
	if (!result)
	{
		return false;
	}

	return true;
}

bool GraphicsClass::GenerateScreenBuffer()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT result;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;

	if (screenBuffer)
	{
		screenBuffer->Release();
		screenBuffer = nullptr;
	}

	if (screenTargetView)
	{
		screenTargetView->Release();
		screenTargetView = nullptr;
	}

	if (depthBuffer)
	{
		depthBuffer->Release();
		depthBuffer = nullptr;
	}

	if (depthTargetView)
	{
		depthTargetView->Release();
		depthTargetView = nullptr;
	}

	ZeroMemory(&textureDesc, sizeof(textureDesc));

	//Setup Render target texture desc
	textureDesc.Width = currentScreenWidth;
	textureDesc.Height = currentScreenHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	//Multisample
	textureDesc.SampleDesc.Count = direct3D->GetCurrentSampleCount();
	textureDesc.SampleDesc.Quality = direct3D->GetCurrentQualityLevel();

	// Create the render target texture.
	result = direct3D->GetDevice()->CreateTexture2D(&textureDesc, nullptr, &screenBuffer);
	if (FAILED(result))
	{
		return false;
	}

	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DMS;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	result = direct3D->GetDevice()->CreateRenderTargetView(screenBuffer, &renderTargetViewDesc, &screenTargetView);
	if (FAILED(result))
	{
		return false;
	}

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = currentScreenWidth;
	depthBufferDesc.Height = currentScreenHeight;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	//Multisample
	depthBufferDesc.SampleDesc.Count = direct3D->GetCurrentSampleCount();
	depthBufferDesc.SampleDesc.Quality = direct3D->GetCurrentQualityLevel();

	// Create the texture for the depth buffer using the filled out description.
	result = direct3D->GetDevice()->CreateTexture2D(&depthBufferDesc, nullptr, &depthBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	result = direct3D->GetDevice()->CreateDepthStencilView(depthBuffer, &depthStencilViewDesc, &depthTargetView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void GraphicsClass::SetupPrimitiveBatch()
{
	primitiveBatch = new PrimitiveBatch<VertexPositionColor>(direct3D->GetDeviceContext());

	basicEffect = new BasicEffect(direct3D->GetDevice());
	basicEffect->SetVertexColorEnabled(true);

	void const* shaderByteCode;
	size_t byteCodeLength;

	basicEffect->GetVertexShaderBytecode(&shaderByteCode, &byteCodeLength);

	direct3D->GetDevice()->CreateInputLayout(VertexPositionColor::InputElements,
		VertexPositionColor::InputElementCount,
		shaderByteCode, byteCodeLength,
		&inputLayout);
}

bool GraphicsClass::RenderSceneToTexture()
{
	XMMATRIX lightViewMatrix, lightProjectionMatrix;

	//Set texture as render target
	renderTexture->SetRenderTarget(direct3D->GetDeviceContext());

	//Clear rendertexture
	renderTexture->ClearRenderTarget(direct3D->GetDeviceContext(), 0.0f, 0.0f, 0.0f, 1.0f);

	light->GenerateViewMatrix();

	light->GetViewMatrix(lightViewMatrix);
	light->GetProjectionMatrix(lightProjectionMatrix);

	//Geometry Goes here, Shadow Map RenderPass

	direct3D->SetBackBufferRenderTarget();
	direct3D->ResetViewport();

	return true;
}

bool GraphicsClass::SetScreenBuffer(float red, float green, float blue, float alpha)
{
	XMMATRIX lightViewMatrix, lightProjectionMatrix;

	direct3D->BeginScene(red, green, blue, alpha);

	//Set texture as render target
	renderTexture->SetRenderTarget(direct3D->GetDeviceContext());
	direct3D->GetDeviceContext()->OMSetRenderTargets(1, &screenTargetView, depthTargetView);
	direct3D->GetDeviceContext()->RSSetViewports(1, &direct3D->viewport);

	//Clear rendertexture
	float color[4];

	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	direct3D->GetDeviceContext()->ClearRenderTargetView(screenTargetView, color);

	direct3D->GetDeviceContext()->ClearDepthStencilView(depthTargetView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	return true;
}

bool GraphicsClass::Render(float rotation, InputClass* input, float deltaTime)
{
	XMMATRIX viewMatrix, projectionMatrix, translateMatrix;
	XMMATRIX lightViewMatrix, lightProjectionMatrix;

	//Render scene to texture
	//result = RenderSceneToTexture();
	//if (!result)
	//{
	//	return false;
	//}

	//Generate view matrix based on camera
	camera->DoMovement(input);
	camera->Render();

	//Get world, view and proj matrices
	//direct3D->GetWorldMatrix(worldMatrix);
	camera->GetViewMatrix(viewMatrix);
	direct3D->GetProjectionMatrix(projectionMatrix);
	direct3D->GetWorldMatrix(translateMatrix);

	//Lighting
	light->GenerateViewMatrix();
	light->GetViewMatrix(lightViewMatrix);
	light->GetProjectionMatrix(lightProjectionMatrix);
	
	//clear Buffer at beginning
	//direct3D->BeginScene(0.2f, 0.5f, 0.5f, 0.0f);
	SetScreenBuffer(0.5f, 0.5f, 0.5f, 1.0f);

	//Render Geometry	
	if(terrain)
	{
		terrain->Render(direct3D->GetDeviceContext(), viewMatrix, projectionMatrix, camera->GetPosition(), steps_initial, steps_refinement, depthfactor, *light);
	}

	//Render Particles
	if (particles) 
	{
		particles->Render(direct3D->GetDeviceContext(), deltaTime, viewMatrix, projectionMatrix);
	}

	//Text
	ID3D11DepthStencilState* depthstate;
	ID3D11RasterizerState* rsstate;
	direct3D->GetDeviceContext()->OMGetDepthStencilState(&depthstate, nullptr);
	direct3D->GetDeviceContext()->RSGetState(&rsstate);
	direct3D->GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);
	m_spriteBatch->Begin(SpriteSortMode_BackToFront, nullptr, nullptr, depthstate, rsstate);

	RenderText(".", Vector2(currentScreenWidth / 2.0f, currentScreenHeight / 2.0f), true);
	RenderText("FPS: " + std::to_string(static_cast<int>(fps)), fpsPos, false);
	RenderText("SampleCount: " + std::to_string(direct3D->GetCurrentSampleCount()) + " of " + std::to_string(direct3D->GetMaxSampleCount()), scPos, false);
	RenderText("QualityLevel: " + std::to_string(direct3D->GetCurrentQualityLevel()) + " of " + std::to_string(direct3D->GetMaxQualityLevels()), qlPos, false);
	m_spriteBatch->End();

	//Primitive Batch Begin
	CommonStates states(direct3D->GetDevice());

	basicEffect->SetWorld(XMMatrixIdentity());
	basicEffect->SetView(viewMatrix);
	basicEffect->SetProjection(projectionMatrix);
	basicEffect->Apply(direct3D->GetDeviceContext());
	direct3D->GetDeviceContext()->IASetInputLayout(inputLayout);

	//Render KD-Tree
	//primitiveBatch->Begin();
	//tree.Draw(primitiveBatch, Colors::LightGreen);
	//primitiveBatch->End();

	ID3D11Texture2D* backBuffer;
	direct3D->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&backBuffer));
	direct3D->GetDeviceContext()->ResolveSubresource(backBuffer, 0, screenBuffer, 0, DXGI_FORMAT_R8G8B8A8_UNORM);

	//Output Buffer
	direct3D->EndScene();

	return true;
}

void GraphicsClass::CheckWireframe(InputClass* input)
{
	unsigned int zkey = 0x5A;

	if (!wireframeMode && !wireframeKeyToggle)
	{
		if (input->IsKeyDown(zkey))
		{
			wireframeMode = true;
			wireframeKeyToggle = true;
			ChangeFillmode(D3D11_FILL_WIREFRAME);
		}
	}
	else if(wireframeMode && !wireframeKeyToggle)
	{
		if (input->IsKeyDown(zkey))
		{
			wireframeMode = false;
			wireframeKeyToggle = true;
			ChangeFillmode(D3D11_FILL_SOLID);
		}
	}

	if (input->IsKeyUp(zkey))
	{
		wireframeKeyToggle = false;
	}
}

void GraphicsClass::CheckMSKeys(InputClass* input)
{
	unsigned int mkey = 0x4D;
	unsigned int commakey = VK_OEM_COMMA;
	unsigned int periodkey = VK_OEM_PERIOD;

	if (!msmodetoggle)
	{
		if (input->IsKeyDown(commakey))
		{
			direct3D->IncreaseSampleCount();
			msmodetoggle = true;
		}
		if (input->IsKeyDown(periodkey))
		{
			direct3D->IncreaseQualityLevel();
			msmodetoggle = true;
		}
		if (input->IsKeyDown(mkey))
		{
			GenerateScreenBuffer();
			msmodetoggle = true;
		}
	}

	if (input->IsKeyUp(mkey) && input->IsKeyUp(periodkey) && input->IsKeyUp(commakey))
	{
		msmodetoggle = false;
	}
}

void GraphicsClass::CheckTerrainKey(InputClass* input)
{
	unsigned int tkey = 0x54;

	if (!terrainKeyToggle)
	{
		if (input->IsKeyDown(tkey))
		{
			RegenrateTerrain();
			terrainKeyToggle = true;
		}
	}

	if (input->IsKeyUp(tkey))
	{
		terrainKeyToggle = false;
	}
}

void GraphicsClass::CheckRotationKey(InputClass* input)
{
	unsigned int rkey = 0x52;

	if (!rotationKeyToggle)
	{
		if (input->IsKeyDown(rkey))
		{
			rotate = !rotate;
			rotationKeyToggle = true;
		}
	}

	if (input->IsKeyUp(rkey))
	{
		rotationKeyToggle = false;
	}
}

void GraphicsClass::CheckFactorKeys(InputClass* input)
{

	unsigned int ikey = 0x49;
	unsigned int okey = 0x4F;
	unsigned int kkey = 0x4B;
	unsigned int lkey = 0x4C;
	unsigned int ukey = 0x55;
	unsigned int jkey = 0x4A;

	if (!factorKeyToggle)
	{
		if (input->IsKeyDown(okey))
		{
			steps_initial += 5;
		}

		if (input->IsKeyDown(ikey))
		{
			if (steps_initial >= 5)
			{
				steps_initial -= 5;
			}
		}

		if (input->IsKeyDown(lkey))
		{
			steps_refinement += 5;
		}

		if (input->IsKeyDown(kkey))
		{
			if (steps_refinement >= 5)
			{
				steps_refinement -= 5;
			}

		}

		if (input->IsKeyDown(ukey))
		{
			depthfactor += 0.01f;
		}

		if (input->IsKeyDown(jkey))
		{
			depthfactor -= 0.01f;

		}

		factorKeyToggle = true;
	}

	if (input->IsKeyUp(ikey) && input->IsKeyUp(okey) && input->IsKeyUp(kkey) && input->IsKeyUp(lkey) && input->IsKeyUp(ukey) && input->IsKeyUp(jkey))
	{
		factorKeyToggle = false;
	}

}

void GraphicsClass::CastRay(const Ray& ray)
{
	float maxRange = 100000.0f;
	float hitfloat = 100000.0f;

	KdTree::RayHitStruct hit1;
	if(tree.hit(&ray, hitfloat, maxRange, hit1))
	{
		SpawnParticles(hit1.hitPoint.x, hit1.hitPoint.y, hit1.hitPoint.z);
	} else
	{
		printf("Hit nothing.\n\r");
	}

}

void GraphicsClass::CheckRaycast(InputClass* input)
{
	unsigned int spaceKey = VK_SPACE;

	if (!rayToggle)
	{
		if (input->IsKeyDown(spaceKey))
		{
			Ray ray;
			ray.position = camera->GetPosition();
			XMMATRIX newdir;
			camera->GetViewMatrix(newdir);
			ray.direction = Matrix(newdir).Transpose().Backward();
			rayToggle = true;

			CastRay(ray);
		}
	}

	if (rayToggle && input->IsKeyUp(spaceKey))
	{
		rayToggle = false;
	}
}

void GraphicsClass::ChangeFillmode(D3D11_FILL_MODE fillmode)
{
	ID3D11RasterizerState* rasterState;
	D3D11_RASTERIZER_DESC rasterStateDesc;

	direct3D->GetDeviceContext()->RSGetState(&rasterState);
	rasterState->GetDesc(&rasterStateDesc);
	rasterStateDesc.FillMode = fillmode;

	direct3D->GetDevice()->CreateRasterizerState(&rasterStateDesc, &rasterState);
	direct3D->GetDeviceContext()->RSSetState(rasterState);

	rasterState->Release();
	rasterState = nullptr;
}

void GraphicsClass::RenderText(string texttorender, Vector2 screenPos, bool centerOrigin)
{
	wstring test(texttorender.begin(), texttorender.end());
	const wchar_t* output = test.c_str();
	Vector2 origin;

	if(centerOrigin)
	{
		 origin = m_font->MeasureString(output) / 2.f;
	} else
	{
		origin = Vector2::Zero;
	}

	m_font->DrawString(m_spriteBatch.get(), output,
		screenPos, Colors::White, 0.f, origin, 0.5f);
}

void GraphicsClass::RenderText(int inttorender, Vector2 screenPos, bool centerOrigin)
{
	wstring test = std::to_wstring(inttorender);
	const wchar_t* output = test.c_str();

	Vector2 origin;

	if (centerOrigin)
	{
		origin = m_font->MeasureString(output) / 2.f;
	}
	else
	{
		origin = Vector2::Zero;
	}

	m_font->DrawString(m_spriteBatch.get(), output,
		screenPos, Colors::White, 0.f, origin, 0.5f);

}
 
void GraphicsClass::RegenrateTerrain()
{
	tree.PurgeTriangles();
	delete terrain;

	terrain = new GeometryData(64, 64, 64, GeometryData::TerrainType::HELIX, direct3D->GetDevice(), direct3D->GetDeviceContext(), &tree);
	terrain->worldMatrix = XMMatrixIdentity() * XMMatrixScaling(5.0f, 5.0f, 5.0f);
	//terrain->DebugPrint();
}

void GraphicsClass::SpawnParticles(float x, float y, float z)
{
	delete particles;

	particles = new ParticleSystem(direct3D->GetDevice(), x, y, z);
}

/*
 *	TODO Shadows
 *	Use Light Class in TerrainClass Render()
 *	Make Depth Shader (Look at old Project)
 *	Render Geometry to texture, using light projection
 *	Use generated Shadow Map in Pixelshader of terrain etc
 *
 */