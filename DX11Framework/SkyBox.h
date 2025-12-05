#pragma once
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <string>
#include "stb_image.h"

class ModelComponent;
class MatrixBuffer;

class SkyBox
{
public:
	ID3D11ShaderResourceView* m_textureView = nullptr;

	HRESULT Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, HWND winHandle, const char* texturePathArray[6]);

	void Render(ID3D11DeviceContext* devCon, MatrixBuffer& mb, DirectX::XMFLOAT3 camPosition);

private:
	HRESULT InitTextureGrid(ID3D11Device* device, ID3D11DeviceContext* devCon, const char* texturePathArray[6]);
	HRESULT InitShaders(ID3D11Device* device, HWND winHandle);
	HRESULT InitRasterState(ID3D11Device* dev);
	HRESULT InitDepthStencil(ID3D11Device* device);

private:
	int m_width, m_height;

	ID3D11Texture2D* m_texture = nullptr;
	ModelComponent* m_cube = nullptr;

	ID3D11DepthStencilState* m_depthStencil = nullptr;
	ID3D11RasterizerState* m_rasterState;

	ID3D11VertexShader* m_vertShader;
	ID3D11PixelShader* m_pixelShader;
};

