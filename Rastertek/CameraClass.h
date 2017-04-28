#pragma once

#include <d3d11.h>
#include "InputClass.h"
#include "SimpleMath.h"
#include "TimerClass.h"
#include <vector>
using namespace DirectX;
using namespace SimpleMath;

class CameraClass
{
public:
	struct ControlPoint
	{
		Vector3 position;
		Quaternion direction;
	};

	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	bool Initialize(ID3D11Device*);
	void Shutdown();

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetForward();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRotation();

	void DoMovement(InputClass*);

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:


	ID3D11Device* device;
	Vector3 position, rotation;
	Matrix viewMatrix;
	TimerClass* timer;
	Quaternion viewQuaternion;
};