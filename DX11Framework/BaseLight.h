#ifndef BASE_LIGHT_H
#define BASE_LIGHT_H

#include <DirectXMath.h>
#include <d3d11_4.h>
#include "ConstantBuffer.h"

class BaseLight
{
public:
	BaseLight(DirectX::XMFLOAT4 diffuseCol, DirectX::XMFLOAT4 ambientCol,
		DirectX::XMFLOAT4 specularCol, int specularPower);
	~BaseLight();

	bool Initialise(ID3D11Device* device);
	static void Render(ID3D11DeviceContext* devCon);

	float* GetAmbientColour();
	float* GetDiffuseColour();
	float* GetSpecularColour();

	float* GetSpecularPower();

protected:
	int m_lightIndex;
	static int numOfActiveLights;

	static LightVertexBuffer m_lightVertexData;
	static ID3D11Buffer* m_lightVertexBuffer;

	static LightPixelBuffer m_lightPixelData;
	static ID3D11Buffer* m_lightPixelBuffer;
};

#endif // !BASE_LIGHT_H