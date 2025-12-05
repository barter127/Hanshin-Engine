#include "BaseLight.h"

using namespace DirectX;

int BaseLight::numOfActiveLights = 0;
LightPixelBuffer BaseLight::m_lightPixelData = {};
ID3D11Buffer* BaseLight::m_lightVertexBuffer = nullptr;
ID3D11Buffer* BaseLight::m_lightPixelBuffer = nullptr;

BaseLight::BaseLight(XMFLOAT4 diffuseCol, XMFLOAT4 ambientCol,
	XMFLOAT4 specularCol, int specularPower)
{
	m_lightIndex = BaseLight::numOfActiveLights;
	BaseLight::numOfActiveLights++;

	BaseLight::m_lightPixelData.data[m_lightIndex].DiffuseColour = diffuseCol;
	BaseLight::m_lightPixelData.data[m_lightIndex].AmbientColour = ambientCol;
	BaseLight::m_lightPixelData.data[m_lightIndex].SpecularColour = specularCol;
	BaseLight::m_lightPixelData.data[m_lightIndex].SpecularPower = specularPower;

	//BaseLight::m_lightVertexData.data[m_lightIndex].numOfActiveLights;
	BaseLight::m_lightPixelData.numOfActiveLights = numOfActiveLights;
}

bool BaseLight::Initialise(ID3D11Device* device)
{
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(LightPixelBuffer);
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&constantBufferDesc, nullptr, &m_lightPixelBuffer);
	if (FAILED(hr)) return hr;

	constantBufferDesc.ByteWidth = sizeof(LightVertexBuffer);
	hr = device->CreateBuffer(&constantBufferDesc, nullptr, &m_lightVertexBuffer);
	if (FAILED(hr)) return hr;

	return hr;
}

void BaseLight::Render(ID3D11DeviceContext* devCon)
{
	D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};
	devCon->Map(m_lightVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, &m_lightVertexData, sizeof(LightVertexBuffer));
	devCon->Unmap(m_lightVertexBuffer, 0);

	devCon->VSSetConstantBuffers(1, 1, &m_lightVertexBuffer);
	devCon->VSSetConstantBuffers(2, 1, &m_lightPixelBuffer);

	mappedSubresource = {};
	devCon->Map(m_lightPixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, &m_lightPixelData, sizeof(LightPixelBuffer));
	devCon->Unmap(m_lightPixelBuffer, 0);

	devCon->PSSetConstantBuffers(2, 1, &m_lightPixelBuffer);
}

float* BaseLight::GetAmbientColour()
{
	return &m_lightPixelData.data[m_lightIndex].AmbientColour.x;
}

float* BaseLight::GetDiffuseColour()
{
	return &m_lightPixelData.data[m_lightIndex].DiffuseColour.x;
}

float* BaseLight::GetSpecularColour()
{
	return &m_lightPixelData.data[m_lightIndex].SpecularColour.x;
}

float* BaseLight::GetSpecularPower()
{
	return &m_lightPixelData.data[m_lightIndex].SpecularPower;
}

BaseLight::~BaseLight()
{
	if (m_lightPixelBuffer) m_lightPixelBuffer->Release();
}
