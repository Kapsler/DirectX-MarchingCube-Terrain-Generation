#include "LightClass.h"

LightClass::LightClass()
{
}

LightClass::LightClass(const LightClass&)
{
}

LightClass::~LightClass()
{
}

void LightClass::SetAmbientColor(float red, float green, float blue, float alpha)
{
	ambientColor = XMFLOAT4(red, green, blue, alpha);
}

XMFLOAT4 LightClass::GetAmbientColor()
{
	return  XMFLOAT4(ambientColor);
}

void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	diffuseColor = XMFLOAT4(red, green, blue, alpha);
}

void LightClass::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
}

void LightClass::SetLookAt(float x, float y, float z)
{
	lookAt.x = x;
	lookAt.y = y;
	lookAt.z = z;
}

XMFLOAT3 LightClass::GetPosition()
{
	return position;
}

void LightClass::GenerateViewMatrix()
{
	SimpleMath::Vector3 up;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	viewMatrix = XMMatrixLookAtLH(position, lookAt, up);
}

void LightClass::GenerateProjectionsMatrix(float screenDepth, float screenNear)
{
	float fieldOfView, screenAspect;

	//Setup fov and aspect for square light source
	fieldOfView = XM_PIDIV2;
	screenAspect = 1.0f;

	projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);
}

void LightClass::GetViewMatrix(XMMATRIX& otherview)
{
	otherview = viewMatrix;
}

void LightClass::GetProjectionMatrix(XMMATRIX& otherproj)
{
	otherproj = projectionMatrix;
}

XMFLOAT4 LightClass::GetDiffuseColor()
{
	return XMFLOAT4(diffuseColor);
}