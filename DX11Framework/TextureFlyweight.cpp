#include "TextureFlyweight.h"
#include "TextureClass.h"

using namespace std;

unordered_map<size_t, shared_ptr<TextureClass>> Texture_Flyweight::m_textureMap;

std::weak_ptr<TextureClass> Texture_Flyweight::FindTexture(string path, ID3D11Device* device, ID3D11DeviceContext* devCon)
{
	hash<string> hasher;
	size_t key = hasher(path);
	if (m_textureMap.find(key) != m_textureMap.end())
	{
		return m_textureMap[key];
	}
	else
	{
		shared_ptr<TextureClass> sharedTexturePtr = make_shared<TextureClass>();
		sharedTexturePtr->Initialise(device, devCon, (char*)path.c_str());

		m_textureMap.insert({ key, sharedTexturePtr });

		return sharedTexturePtr;
	}
}
