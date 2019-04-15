#include "Light.h"

bool Light::Initialize(ID3D11Device * device, ID3D11DeviceContext * deviceContext, ConstantBuffer<CB_VS_vertexshader>& cb_vs_vertexshader)
{
	if (!model.Initialize("Data\\Objects\\light.fbx", device, deviceContext, cb_vs_vertexshader))
	{ return false; }

	//SetPosition(0.0f, 0.0f, 0.0f);
	//SetRotation(0.0f, 0.0f, 0.0f);
	//RenderableGameObject::UpdateMatrix();

	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	posVector = XMLoadFloat3(&pos);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotVector = XMLoadFloat3(&rot);
	UpdateMatrix();

	return true;
}

void Light::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	projetionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const XMMATRIX & Light::GetViewMatrix() const
{
	return viewMatrix;
}

const XMMATRIX & Light::GetProjectionMatrix() const
{
	return projetionMatrix;
}

void Light::UpdateMatrix()
{
	//unit vector of light target based off of forward vector
	XMVECTOR lightTarget = DEFAULT_FORWARD_VECTOR;
	//up direction
	XMVECTOR upDir = DEFAULT_UP_VECTOR;
	//adjust lights target to be offset by the current position
	lightTarget += posVector;
	//rebuild view matrix
	viewMatrix = XMMatrixLookAtLH(posVector, lightTarget, upDir);
}
