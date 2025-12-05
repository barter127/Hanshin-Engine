#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>

#include "ConstantBuffer.h"
#include "GameObject.h"
#include <wrl.h>

class ApplicationLayer;

struct SimpleVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 Texcoord;
	DirectX::XMFLOAT3 Normal;
};

class DX11Framework
{
public:
	HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);
	HRESULT CreateWindowHandle(HINSTANCE hInstance, int nCmdShow);
	HRESULT CreateD3DDevice();
	HRESULT CreateSwapChainAndFrameBuffer();
	HRESULT InitShadersAndInputLayout();
	HRESULT InitPipelineVariables();
	~DX11Framework();
	void Update();
	void Draw();

	void UIMessage(UINT umessage, WPARAM wparam, LPARAM lparam);

	void SetBackBufferRenderTarget();
	void ResetViewport();

	ID3D11DeviceContext* _immediateContext = nullptr;
private:
	int _WindowWidth = 1280;
	int _WindowHeight = 768;

	ID3D11Device* _device;
	IDXGIDevice* _dxgiDevice = nullptr;
	IDXGIFactory2* _dxgiFactory = nullptr;
	ID3D11RenderTargetView* _frameBufferView = nullptr;
	IDXGISwapChain1* _swapChain;
	D3D11_VIEWPORT _viewport;

	ID3D11Texture2D* _depthStencilBuffer = nullptr;
	ID3D11DepthStencilView* _depthStencilView = nullptr;
	ID3D11DepthStencilState* _depthStencilState = nullptr;

	ID3D11RasterizerState* _rasterizerState;
	ID3D11RasterizerState* _wireframeState;

	ID3D11VertexShader* _vertexShader;
	ID3D11InputLayout* _inputLayout;
	ID3D11PixelShader* _pixelShader;

	HWND _windowHandle;

	DirectX::XMFLOAT4X4 _World;
	DirectX::XMFLOAT4X4 _View;
	DirectX::XMFLOAT4X4 _Projection;

	MatrixBuffer _mbData;
	ApplicationLayer* m_Application;
};