#pragma once

#include "D3DClass.h"
#include "CameraClass.h"
#include "ShaderClass.h"
#include "TextureClass.h"
#include <vector>
#include "LightClass.h"
#include "RenderTextureClass.h"
#include "DepthShaderClass.h"
#include "SpriteFont.h"
#include <PrimitiveBatch.h>
#include <VertexTypes.h>
#include <Effects.h>
#include <CommonStates.h>
#include "PerformanceTimer.h"
#include "GeometryData.h"

const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 2000.0f;
const float SCREEN_NEAR = 0.01f;
const int SHADOWMAP_WIDTH = 2048;
const int SHADOWMAP_HEIGHT = 2048;

class GraphicsClass
{
public:

	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(InputClass*);

	bool GenerateScreenBuffer();

private:
	void SetupPrimitiveBatch();
	bool RenderSceneToTexture();
	bool SetScreenBuffer(float, float, float, float);
	bool Render(float, InputClass*);
	void CheckWireframe(InputClass*);
	void CheckMSKeys(InputClass*);
	void CheckTerrainKey(InputClass* input);
	void CheckRotationKey(InputClass* input);
	void CheckFactorKeys(InputClass* input);
	void SetLightDirection(InputClass*);
	void ChangeFillmode(D3D11_FILL_MODE);
	void RenderText(string texttorender, Vector2 screenPos, bool centerOrigin);
	void RenderText(int inttorender, Vector2 screenPos, bool centerOrigin);
	void RegenrateTerrain();

	D3DClass* direct3D;
	CameraClass* camera;
	ShaderClass* shader;
	HWND* hwndptr;
	LightClass* light;
	TimerClass* timer;
	RenderTextureClass* renderTexture;
	DepthShaderClass* depthShader;

	bool wireframeMode, wireframeKeyToggle = false, bumpinessKeyToggle = false, msmodetoggle = false, terrainKeyToggle = false;
	bool rotationKeyToggle = false;
	bool factorKeyToggle = false;
	bool rotate = false;
	float bumpiness;
	int currentScreenWidth, currentScreenHeight;
	float fps;
	int steps_initial = 10;
	int steps_refinement = 5;
	float depthfactor = 0.08f;

	unique_ptr<DirectX::SpriteFont> m_font;
	DirectX::SimpleMath::Vector2 fpsPos, scPos, qlPos;
	std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

	ID3D11Texture2D* screenBuffer;
	ID3D11RenderTargetView* screenTargetView;
	ID3D11Texture2D* depthBuffer;
	ID3D11DepthStencilView* depthTargetView;

	PrimitiveBatch<VertexPositionColor>* primitiveBatch; 
	BasicEffect* basicEffect;
	ID3D11InputLayout* inputLayout;
	
	GeometryData* terrain = nullptr;
};