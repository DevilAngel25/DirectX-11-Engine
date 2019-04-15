#include "Camera3D.h"

Camera3D::Camera3D()
{
	pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
	posVector = XMLoadFloat3(&pos);
	rot = XMFLOAT3(0.0f, 0.0f, 0.0f);
	rotVector = XMLoadFloat3(&rot);
	UpdateMatrix();
}

void Camera3D::SetProjectionValues(float fovDegrees, float aspectRatio, float nearZ, float farZ)
{
	float fovRadians = (fovDegrees / 360.0f) * XM_2PI;
	projetionMatrix = XMMatrixPerspectiveFovLH(fovRadians, aspectRatio, nearZ, farZ);
}

const XMMATRIX & Camera3D::GetViewMatrix() const
{
	return viewMatrix;
}

const XMMATRIX & Camera3D::GetProjectionMatrix() const
{
	return projetionMatrix;
}

void Camera3D::UpdateMatrix()
{
	//calculate camera rotation matrix
	XMMATRIX camRotationMatrix = XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
	//calculate unit vector of cam target based off camera forward value transformed by camera rotation matrix
	XMVECTOR camTarget = XMVector3TransformCoord(DEFAULT_FORWARD_VECTOR, camRotationMatrix);
	//adjust cam target to be offset by the camera's current position
	camTarget += posVector;
	//calculate up direction based on current rotation
	XMVECTOR upDir = XMVector3TransformCoord(DEFAULT_UP_VECTOR, camRotationMatrix);
	//rebuild view matrix
	viewMatrix = XMMatrixLookAtLH(posVector, camTarget, upDir);

	UpdateDirectionVectors();
}
