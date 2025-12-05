#pragma once
#include <DirectXMath.h>

#define NUM_LIGHTS 8

#define DIRECTIONAL_LIGHT 0
#define POINT_LIGHT 1
#define SPOT_LIGHT 2

struct MatrixBuffer
{
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX World;
};

struct LightData
{
	DirectX::XMFLOAT4 DiffuseColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 AmbientColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT4 SpecularColour = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	DirectX::XMFLOAT2 padding;
	int lightType = POINT_LIGHT;
	float SpecularPower;

	DirectX::XMFLOAT3 LightDirection = DirectX::XMFLOAT3(0, 0, 0);
	float falloffMultiplier = 0.2f; // If low less falloff if high more falloff. Please don't set to 0.
};

struct LightPixelBuffer
{
	LightData data[NUM_LIGHTS];

	DirectX::XMFLOAT3 CameraPos;
	int numOfActiveLights = 2;
};

struct LightVertexBuffer
{
	// FLoat4 for padding.
	DirectX::XMFLOAT4 LightPosition[NUM_LIGHTS];
};

struct GaussianData
{
	DirectX::XMFLOAT2 WindowDimensions = { 1280.0f, 768.0f };
	int KernelSize = 10;
	float Sigma = 10.0f;
	float SamplePosMultiplier = 2;
	DirectX::XMFLOAT3 Padding;
};