#include "CameraClass.h"
#include <iostream>

CameraClass::CameraClass()
{
	position.x = 0.0f;
	position.y = 0.0f;
	position.z = 0.0f;

	rotation.x = 0.0f;
	rotation.y = 0.0f;
	rotation.z = 0.0f;

}

CameraClass::CameraClass(const CameraClass&)
{
}

CameraClass::~CameraClass()
{
}

void CameraClass::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y= y;
	position.z = z;	
}

void CameraClass::SetRotation(float x, float y, float z)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
}

bool CameraClass::Initialize(ID3D11Device* odevice)
{
	bool result;

	device = odevice;

	timer = new TimerClass();
	if(!timer)
	{
		return false;
	}

	result = timer->Initialize();
	if(!result)
	{
		return false;
	}

	return true;
}

void CameraClass::Shutdown()
{	
	if(timer)
	{
		delete timer;
		timer = nullptr;
	}
}

XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(position);
}

XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(rotation);
}

void CameraClass::DoMovement(InputClass* input)
{
	unsigned int wkey = 0x57;
	unsigned int skey = 0x53;
	unsigned int akey = 0x41;
	unsigned int dkey = 0x44;
	Vector3 movementDirection;
	timer->Frame();


	float deltaTime = timer->GetFrameTime();
	float cameraSpeed = 0.005f * deltaTime;
	float rotationSpeed = 0.1f * deltaTime;
	viewQuaternion.Inverse(viewQuaternion);

		//Movement
		if (input->IsKeyDown(wkey))
		{
			movementDirection = Vector3::Transform(Vector3::Forward, viewQuaternion);
			position -=  movementDirection * cameraSpeed;
			
		}
		if (input->IsKeyDown(skey))
		{
			movementDirection = Vector3::Transform(Vector3::Forward, viewQuaternion);
			position += cameraSpeed * movementDirection;
		}
		if (input->IsKeyDown(akey))
		{
			movementDirection = Vector3::Transform(Vector3::Forward.Cross(Vector3::Up), viewQuaternion);
			position -= cameraSpeed * movementDirection;
		}
		if (input->IsKeyDown(dkey))
		{
			movementDirection = Vector3::Transform(Vector3::Forward.Cross(Vector3::Up), viewQuaternion);
			position += cameraSpeed * movementDirection;
		}

		//Rotation
		if(input->IsKeyDown(VK_UP))
		{
			rotation.x -= rotationSpeed * 0.5f ;
		}
		if(input->IsKeyDown(VK_DOWN))
		{
			rotation.x += rotationSpeed * 0.5f;
		}
		if(input->IsKeyDown(VK_LEFT))
		{
			rotation.y -= rotationSpeed * 1;
		}
		if(input->IsKeyDown(VK_RIGHT))
		{
			rotation.y += rotationSpeed * 1;
		}


}

void CameraClass::Render()
{
	float yaw, pitch, roll;

	//Set Rotation in radians
	pitch = rotation.x * 0.0174532925f;
	yaw = rotation.y * 0.0174532925f;
	roll = rotation.z * 0.0174532925f;

	viewQuaternion = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);


	//Finally create view matrix
	//viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
	viewQuaternion.Inverse(viewQuaternion);
	viewMatrix = Matrix::Identity;
	//Translate to position of viewer
	viewMatrix = viewMatrix.Transform(Matrix::CreateTranslation(-position), viewQuaternion);
}

void CameraClass::GetViewMatrix(XMMATRIX& output)
{
	output = viewMatrix;
}