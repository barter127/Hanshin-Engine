#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "BaseLight.h"

class PointLight : public BaseLight
{
public:
	PointLight(DirectX::XMFLOAT4 diffuseCol, DirectX::XMFLOAT4 ambientCol,
		DirectX::XMFLOAT4 specularCol, int specularPower,
		DirectX::XMFLOAT3 lightPos);
	~PointLight();

	DirectX::XMFLOAT4& GetLightPos();
};

#endif // !POINT_LIGHT_H