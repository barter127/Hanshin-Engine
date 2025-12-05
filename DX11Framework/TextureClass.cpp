#include "TextureClass.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextureClass::TextureClass()
{
	m_texture = nullptr;
	m_textureView = nullptr;
}

TextureClass::~TextureClass() {}

bool TextureClass::Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	int bpp;
	static const int STBI_RGBA_CHANNEL = 4;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	unsigned char* imageData;

	// Load the image data into memory.
	imageData = stbi_load(filename, &m_width, &m_height, &bpp, STBI_RGBA_CHANNEL);
	if (imageData == nullptr)
	{
		std::string failureReason = stbi_failure_reason();
		return false;
	}

	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create an empty texture.
	hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
	{
		return false;
	}

	// Set the row pitch of the image data.
	rowPitch = (m_width * bpp) * sizeof(unsigned char);

	// Copy the targa image data into the texture.
	deviceContext->UpdateSubresource(m_texture, 0, NULL, imageData, rowPitch, 0);

	// Setup the shader resource view description.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return false;
	}

	// Generate mipmaps for this texture.
	deviceContext->GenerateMips(m_textureView);

	// Deallocate data
	stbi_image_free(imageData);
	return true;
}

void TextureClass::Shutdown()
{
	// Release the texture view resource.
	if (m_textureView)
	{
		m_textureView->Release();
		m_textureView = nullptr;
	}

	// Release the texture.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	return;
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_textureView;
}

int TextureClass::GetWidth()
{
	return m_width;
}


int TextureClass::GetHeight()
{
	return m_height;
}