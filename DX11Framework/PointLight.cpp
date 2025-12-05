#include "PointLight.h"

using namespace DirectX;

LightVertexBuffer PointLight::m_lightVertexData = {};

PointLight::PointLight(XMFLOAT4 diffuseCol, XMFLOAT4 ambientCol,
	XMFLOAT4 specularCol, int specularPower,
	XMFLOAT3 lightPos) :

	BaseLight(diffuseCol, ambientCol, specularCol, specularPower)
{
	XMFLOAT4 pos = XMFLOAT4(lightPos.x, lightPos.y, lightPos.z, 0.0f);
	PointLight::m_lightVertexData.LightPosition[m_lightIndex] = pos;
	m_lightPixelData.data[m_lightIndex].lightType = POINT_LIGHT;
}

PointLight::~PointLight()
{
}

XMFLOAT4& PointLight::GetLightPos()
{
	return m_lightVertexData.LightPosition[m_lightIndex];
}
