#include "SkyBox.h"
#include "ConstantBuffer.h"
#include "ModelComponent.h"

using namespace DirectX;

HRESULT SkyBox::Initialise(ID3D11Device* device, ID3D11DeviceContext* devCon, HWND winHandle, const char* texturePathArray[6])
{
	HRESULT hr = S_OK;

	hr = InitTextureGrid(device, devCon, texturePathArray);
	hr = InitShaders(device, winHandle);
	hr = InitRasterState(device);
	hr = InitDepthStencil(device);

	m_cube = new ModelComponent();
	bool result = m_cube->Initialise(device, devCon, (char*)"Primitives/Prim_Cube.obj");

	return hr;
}

void SkyBox::Render(ID3D11DeviceContext* devCon, MatrixBuffer& mb, XMFLOAT3 camPosition)
{
	devCon->VSSetShader(m_vertShader, nullptr, 0);
	devCon->PSSetShader(m_pixelShader, nullptr, 0);

	devCon->OMSetDepthStencilState(m_depthStencil, 0);
	devCon->RSSetState(m_rasterState);

	devCon->PSSetShaderResources(0, 1, &m_textureView);

	mb.World = XMMatrixTranspose(XMMatrixScaling(10,10,10) * XMMatrixTranslation(camPosition.x, camPosition.y, camPosition.z));
	m_cube->Render(devCon, mb);
}

HRESULT SkyBox::InitTextureGrid(ID3D11Device* device, ID3D11DeviceContext* devCon, const char* texturePathArray[6])
{
	HRESULT hr = S_OK;

	int bpp;
	constexpr int STBI_RGBA_CHANNEL = 4;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	unsigned char* imageData[6];

	// Could use size of here but that feels unnessecary.
	for (int i = 0; i < 6; i++)
	{
		imageData[i] = stbi_load(texturePathArray[i], &m_width, &m_height, &bpp, STBI_RGBA_CHANNEL);
		if (imageData[i] == nullptr)
		{
			std::string failureReason = stbi_failure_reason();
			return S_FALSE;
		}
	}

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	rowPitch = (m_width * STBI_RGBA_CHANNEL);

	D3D11_SUBRESOURCE_DATA pData[6];
	for (int i = 0; i < 6; i++)
	{
		//Pointer to the pixel data
		pData[i].pSysMem = imageData[i];
		//Line width in bytes
		pData[i].SysMemPitch = rowPitch;
		// This is only used for 3d textures.
		pData[i].SysMemSlicePitch = 0;
	}

	// Create an empty texture.
	hResult = device->CreateTexture2D(&textureDesc, &pData[0], &m_texture);
	if (FAILED(hResult))
	{
		return hr;
	}

	// Setup the shader resource view description.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return hr;
	}

	for (int i = 0; i < 6; i++)
	{
		stbi_image_free(imageData[i]);
	}

	return hr;
}

HRESULT SkyBox::InitShaders(ID3D11Device* device, HWND winHandle)
{
	HRESULT hr = S_OK;

	ID3DBlob* errorBlob;
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;

	ID3DBlob* vsBlob;
	ID3DBlob* psBlob;

	hr = D3DCompileFromFile(L"Skybox.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_main", "vs_5_0", dwShaderFlags, 0, &vsBlob, &errorBlob);
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


	hr = D3DCompileFromFile(L"Skybox.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_main", "ps_5_0", dwShaderFlags, 0, &psBlob, &errorBlob);
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

HRESULT SkyBox::InitRasterState(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	// Skybox rasterizer
	D3D11_RASTERIZER_DESC skyboxDesc = {};
	skyboxDesc.FillMode = D3D11_FILL_SOLID;
	skyboxDesc.CullMode = D3D11_CULL_NONE;

	hr = device->CreateRasterizerState(&skyboxDesc, &m_rasterState);
	if (FAILED(hr)) return hr;

	return hr;
}

HRESULT SkyBox::InitDepthStencil(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	D3D11_DEPTH_STENCIL_DESC dsSkyboxDesc = { };
	dsSkyboxDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	dsSkyboxDesc.DepthEnable = true;
	dsSkyboxDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;

	hr = device->CreateDepthStencilState(&dsSkyboxDesc, &m_depthStencil);

	return hr;
}