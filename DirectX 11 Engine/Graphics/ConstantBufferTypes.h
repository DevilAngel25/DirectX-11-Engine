#pragma once
#include <DirectXMath.h>

struct CB_VS_vertexshader
{
	DirectX::XMMATRIX wvpMatrix;
	DirectX::XMMATRIX worldMatrix;
};

struct CB_VS_vertexshader_2d
{
	DirectX::XMMATRIX wvpMatrix;
};

struct CB_PS_light
{
	DirectX::XMFLOAT3 ambientColor;
	float ambientStrength;

	DirectX::XMFLOAT3 diffuseColor;
	float lightStrength;

	DirectX::XMFLOAT3 lightPos;
	float lightAttenuation_a;

	float lightAttenuation_b;
	float lightAttenuation_c;
};
