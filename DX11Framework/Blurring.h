#pragma once
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "ConstantBuffer.h"

#include "RenderTextureClass.h"

class Blurring
{
public:
	Blurring(ID3D11Device* device, ID3D11DeviceContext* deviceCon, HWND winHandle);
	~Blurring();

	void CopyViewport(ID3D11Device* device, ID3D11DeviceContext* devCon, ID3D11ShaderResourceView* srv);

	void DualKawase(ID3D11DeviceContext* devCon);
	void Gaussian(ID3D11DeviceContext* devCon);

	ID3D11ShaderResourceView* GetShaderResourceView();

	GaussianData m_gaussData;

private:
	HRESULT InitShaders(ID3D11Device* device, HWND winHandle);
	HRESULT InitBuffers(ID3D11Device* device);

	RenderTextureClass* m_renderTexture;
	
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	ID3D11VertexShader* m_vertShader;
	ID3D11PixelShader* m_pixelShader;

	MatrixBuffer m_matrixData;
	ID3D11Buffer* m_matrixBuffer;

	ID3D11Buffer* m_gaussBuffer;

	// Special depth stencil so models don't peek through. 
	ID3D11DepthStencilState* m_depthStencil;

	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_projection;
};

