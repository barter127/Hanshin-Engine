#pragma once

#ifndef TEXTURE_CLASS_H
#define TEXTURE_CLASS_H

#include <d3d11.h>
#include <stdio.h>
#include <string>
class TextureClass
{
public:
	TextureClass();
	~TextureClass();

	/*
	* \brief Reads image data from path using stbi and creates DX11 Texture .
	* \returns True if initalised successfully. Returns false if any part of initialisation fails.
	*/
	bool Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceCon, char* filePath);

	// \brief Releases the texture and texture view and sets them to nullptr.
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

	int GetWidth();
	int GetHeight();

	ID3D11ShaderResourceView* m_textureView;
	std::string path;
private:
	ID3D11Texture2D* m_texture;
	int m_width, m_height;
};
#endif

