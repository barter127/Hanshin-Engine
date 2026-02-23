#pragma once

#include <memory>
#include <unordered_map>
#include <string>

#include <d3d11.h>

class TextureClass;

class Texture_Flyweight
{
public:
	static std::weak_ptr<TextureClass> FindTexture(std::string path, ID3D11Device* device, ID3D11DeviceContext* devCon);

private:
	static std::unordered_map<size_t, std::shared_ptr<TextureClass>> m_textureMap;
};