#include "Blurring.h"

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT2 uv;
	XMFLOAT3 normal;
};

Blurring::Blurring(ID3D11Device* device, ID3D11DeviceContext* deviceCon, HWND winHandle)
{
	InitShaders(device, winHandle);
	InitBuffers(device);

	m_renderTexture = new RenderTextureClass();
	m_renderTexture->Initialise(device, 1280.0f, 768.0f, 0.3f, 1000.0f, 1); // Hardcoded :<

	D3D11_DEPTH_STENCIL_DESC dsDesc = { };
	dsDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	device->CreateDepthStencilState(&dsDesc, &m_depthStencil);
}

Blurring::~Blurring()
{
}

void Blurring::CopyViewport(ID3D11Device* device, ID3D11DeviceContext* devCon, ID3D11ShaderResourceView* srv)
{
	m_renderTexture->CopyResource(device, devCon, srv); // This is going to be slow I wonder if there's a more efficient way.
}

void Blurring::DualKawase(ID3D11DeviceContext* devCon)
{
}

void Blurring::Gaussian(ID3D11DeviceContext* devCon)
{
	m_matrixData.World = XMMatrixTranspose(XMLoadFloat4x4(&m_world));
	m_matrixData.View = XMMatrixTranspose(XMLoadFloat4x4(&m_view));
	m_matrixData.Projection = XMMatrixTranspose(XMLoadFloat4x4(&m_projection));

	devCon->VSSetShader(m_vertShader, nullptr, 0);
	devCon->PSSetShader(m_pixelShader, nullptr, 0);

	D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};
	devCon->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, &m_matrixData, sizeof(MatrixBuffer));
	devCon->Unmap(m_matrixBuffer, 0);

	mappedSubresource = {};
	devCon->Map(m_gaussBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	memcpy(mappedSubresource.pData, &m_gaussData, sizeof(GaussianData));
	devCon->Unmap(m_gaussBuffer, 0);

	devCon->OMSetDepthStencilState(m_depthStencil, 1);

	UINT stride = { sizeof(Vertex) };
	UINT offset = 0;
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	devCon->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	devCon->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	auto srvPtr = m_renderTexture->GetShaderResourceView();
	devCon->VSSetConstantBuffers(0, 1, &m_matrixBuffer);
	devCon->PSSetConstantBuffers(1, 1, &m_gaussBuffer);
	devCon->PSSetShaderResources(0, 1, &srvPtr);

	devCon->DrawIndexed(6, 0, 0);
}

ID3D11ShaderResourceView* Blurring::GetShaderResourceView()
{
	return m_renderTexture->GetShaderResourceView();
}

HRESULT Blurring::InitShaders(ID3D11Device* device, HWND winHandle)
{
	HRESULT hr = S_OK;

	ID3DBlob* errorBlob;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ID3DBlob* psBlob;
	ID3DBlob* vsBlob;

	hr = D3DCompileFromFile(L"GaussianBlur.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
	if (FAILED(hr))
	{
		MessageBoxA(winHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
		errorBlob->Release();
		return hr;
	}

	hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &m_vertShader);
	if (FAILED(hr))
	{
		MessageBoxA(winHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
		errorBlob->Release();
		return hr;
	}

	hr = D3DCompileFromFile(L"GaussianBlur.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", dwShaderFlags, 0, &psBlob, &errorBlob);
	if (FAILED(hr))
	{
		MessageBoxA(winHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
		errorBlob->Release();
		return hr;
	}

	hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &m_pixelShader);
	if (FAILED(hr))
	{
		MessageBoxA(winHandle, (char*)errorBlob->GetBufferPointer(), nullptr, ERROR);
		errorBlob->Release();
		return hr;
	}

	vsBlob->Release();
	psBlob->Release();

	return hr;
}

HRESULT Blurring::InitBuffers(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	Vertex planeVertices[] =
	{
		{ {-0.5f, -0.5f, 1.0f}, {0,1}, {0,0,-1} },
		{ {-0.5f,  0.5f, 1.0f}, {0,0}, {0,0,-1} },
		{ { 0.5f,  0.5f, 1.0f}, {1,0}, {0,0,-1} },
		{ { 0.5f, -0.5f, 1.0f}, {1,1}, {0,0,-1} }
	};

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(planeVertices);
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = planeVertices;

	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	unsigned int planeIndices[] =
	{
		0, 1, 2,
		0, 2, 3
	};

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(planeIndices);
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = planeIndices;

	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = device->CreateBuffer(&constantBufferDesc, nullptr, &m_matrixBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	D3D11_BUFFER_DESC gaussBufferDesc = {};
	gaussBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	gaussBufferDesc.ByteWidth = sizeof(GaussianData);
	gaussBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	gaussBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = device->CreateBuffer(&gaussBufferDesc, nullptr, &m_gaussBuffer);
	if (FAILED(hr))
	{
		return hr;
	}

	// Put plane just infront of camera.
	XMStoreFloat4x4(&m_world, XMMatrixIdentity() * XMMatrixScaling(10.0f, 10.0f, 10.0f) * XMMatrixTranslation(0, 0, -7));

	XMFLOAT3 Eye = XMFLOAT3(0, 0, -0.001f);
	XMFLOAT3 At = XMFLOAT3(0, 0, 0);
	XMFLOAT3 Up = XMFLOAT3(0, 1, 0);

	float aspect = 1280.0f / 768.0f; // Hardcoded :<
	XMStoreFloat4x4(&m_view, XMMatrixLookAtLH(XMLoadFloat3(&Eye), XMLoadFloat3(&At), XMLoadFloat3(&Up)));

	XMMATRIX perspective = XMMatrixPerspectiveFovLH(XMConvertToRadians(90), aspect, 0.01f, 100.0f);
	XMStoreFloat4x4(&m_projection, perspective);

	return hr;
}