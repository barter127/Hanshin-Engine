#pragma once

#include <windows.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <memory>

#include "ConstantBuffer.h"
#include "ApplicationLayer.h"
#include <wrl.h>


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

	void SetBackBufferRenderTarget();
	void ResetViewport();

	ID3D11DeviceContext* m_immediateContext = nullptr;
private:
	int m_windowWidth = 1280;
	int m_windowHeight = 768;

	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr <IDXGIDevice> m_dxgiDevice = nullptr;
	Microsoft::WRL::ComPtr<IDXGIFactory2> m_dxgiFactory = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_frameBufferView = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain1> m_swapChain;
	D3D11_VIEWPORT m_viewport;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView = nullptr;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState = nullptr;

	Microsoft::WRL::ComPtr <ID3D11RasterizerState> m_rasterizerState = nullptr;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_wireframeState = nullptr;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader = nullptr;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout = nullptr;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader = nullptr;

	HWND m_windowHandle;

	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_projection;

	MatrixBuffer m_mbData;
	std::unique_ptr<ApplicationLayer> m_Application;
};