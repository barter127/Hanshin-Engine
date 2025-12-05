#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "BaseLight.h"

class DirectionalLight : public BaseLight
{
public:
	DirectionalLight(DirectX::XMFLOAT4 diffuseCol, DirectX::XMFLOAT4 ambientCol,
		DirectX::XMFLOAT4 specularCol, int specularPower,
		DirectX::XMFLOAT3 lightDir);
	~DirectionalLight();

	// Need to figure out a good way to edit the colour values.

	DirectX::XMFLOAT4& GetLightDirection();
};

#endif // !DIRECTIONAL_LIGHT_H