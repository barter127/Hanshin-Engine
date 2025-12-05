#include "DirectionalLight.h"

using namespace DirectX;

DirectionalLight::DirectionalLight(XMFLOAT4 diffuseCol, XMFLOAT4 ambientCol, 
	XMFLOAT4 specularCol, int specularPower,
	XMFLOAT3 lightDir) : 

	BaseLight(diffuseCol, ambientCol, specularCol, specularPower)
{
	XMFLOAT4 dir = XMFLOAT4(lightDir.x, lightDir.y, lightDir.z, 0);
	m_lightVertexData.LightPosition[m_lightIndex] = dir;

	m_lightPixelData.data[m_lightIndex].LightDirection = lightDir;
	m_lightPixelData.data[m_lightIndex].lightType = DIRECTIONAL_LIGHT;
}

DirectionalLight::~DirectionalLight()
{
}

XMFLOAT4& DirectionalLight::GetLightDirection()
{
	return m_lightVertexData.LightPosition[m_lightIndex];
}
