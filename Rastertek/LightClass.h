#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <SimpleMath.h>
using namespace DirectX;

class LightClass
{
public:
	LightClass();
	LightClass(const LightClass&);
	~LightClass();

	void SetAmbientColor(float, float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetPosition(float, float, float);
	void SetLookAt(float, float, float);

	XMFLOAT4 GetAmbientColor();
	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetPosition();
	XMFLOAT4 GetDirection();

	void GenerateViewMatrix();
	void GenerateProjectionsMatrix(float, float);

	void GetViewMatrix(XMMATRIX&);
	void GetProjectionMatrix(XMMATRIX&);

private:
	SimpleMath::Vector4 diffuseColor, ambientColor;
	SimpleMath::Vector3 position, lookAt;
	XMMATRIX viewMatrix, projectionMatrix;
};
