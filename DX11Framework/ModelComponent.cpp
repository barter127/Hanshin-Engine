#include "ModelComponent.h"

#include "TextureClass.h"
#include "Mesh.h"
#include <fstream>
#include <string>
#include <iostream>
#include <vector>

using namespace DirectX;
using namespace std;

struct Texture {
	std::string path;
	ID3D11ShaderResourceView* texture;
};

ModelComponent::ModelComponent() {}

ModelComponent::ModelComponent(const ModelComponent& other) {}

ModelComponent::~ModelComponent() 
{
	Shutdown();
}

bool ModelComponent::Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* modelFilename) 
{
	m_device = device;
	m_deviceCon = deviceContext;

	bool result;

	// Load in the model data.
	result = LoadModel(modelFilename);
	if (!result) { return false; }

	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = device->CreateBuffer(&constantBufferDesc, nullptr, &m_constantBuffer);
	if (FAILED(hr)) { return hr; }

	m_initialised = true;
	return true;
}

void ModelComponent::Shutdown()
{
	for (int i = 0; i < m_meshes.size(); i++)
	{
		m_meshes[i].Release();
	}
}

void ModelComponent::Render(ID3D11DeviceContext* deviceContext, MatrixBuffer& mb)
{
	if (m_initialised)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};
		deviceContext->Map(m_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		memcpy(mappedSubresource.pData, &mb, sizeof(MatrixBuffer));
		deviceContext->Unmap(m_constantBuffer, 0);

		deviceContext->VSSetConstantBuffers(0, 1, &m_constantBuffer);
		//deviceContext->PSSetConstantBuffers(0, 1, &m_constantBuffer); Matricies are currently not needed on ps.

		for (UINT i = 0; i < m_meshes.size(); i++)
		{
			m_meshes[i].Draw(deviceContext);
		}
	}
}

int ModelComponent::GetIndexCount()
{
	return m_indexCount;
}

void ModelComponent::ProcessNode(aiNode* node, const aiScene* scene)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.emplace_back(ProcessMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		ProcessNode(node->mChildren[i], scene);
	}
}

Mesh ModelComponent::ProcessMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<VertexType> vertices;
	vector<UINT> indices;
	vector<TextureClass> textures;

	// Walk through each of the mesh's vertices.
	for (UINT i = 0; i < mesh->mNumVertices; i++) 
	{
		VertexType vertex;

		vertex.position.x = mesh->mVertices[i].x;
		vertex.position.y = mesh->mVertices[i].y;
		vertex.position.z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0]) 
		{
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		if (mesh->mNormals)
		{
			vertex.normal.x = (float)mesh->mNormals[i].x;
			vertex.normal.y = (float)mesh->mNormals[i].y;
			vertex.normal.z = (float)mesh->mNormals[i].z;
		}

		vertices.emplace_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		m_indexCount += face.mNumIndices; // Maybe a minor optimisation here.

		for (UINT j = 0; j < face.mNumIndices; j++)
		{
			indices.emplace_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0) 
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];


		std::vector<TextureClass> textureList = LoadMaterialTextures(material, aiTextureType_DIFFUSE);
		for (TextureClass texture : textureList)
		{
			textures.push_back(texture);
		}
		textureList = LoadMaterialTextures(material, aiTextureType_HEIGHT);
		for (TextureClass texture : textureList)
		{
			textures.push_back(texture);
		}
		textureList = LoadMaterialTextures(material, aiTextureType_SHININESS);
		for (TextureClass texture : textureList)
		{
			textures.push_back(texture);
		}
		textureList = LoadMaterialTextures(material, aiTextureType_AMBIENT);
		for (TextureClass texture : textureList)
		{
			textures.push_back(texture);
		}


		//textureList = LoadMaterialTextures(material, aiTextureType_HEIGHT);
		//for (auto texture : textureList)
		//{
		//	std::weak_ptr foundModel = Texture_Flyweight::FindTexture(texture, m_device, m_deviceCon);
		//	std::shared_ptr<TextureClass> foundModelShared = foundModel.lock();

		//	if (foundModelShared)
		//	{
		//		textures.push_back(*foundModelShared.get());
		//	}

		//}

		//textureList = LoadMaterialTextures(material, aiTextureType_SHININESS);
		//for (auto texture : textureList)
		//{
		//	std::weak_ptr foundModel = Texture_Flyweight::FindTexture(texture, m_device, m_deviceCon);
		//	std::shared_ptr<TextureClass> foundModelShared = foundModel.lock();

		//	if (foundModelShared)
		//	{
		//		textures.push_back(foundModelShared.get());
		//	}

		//}

		//textureList = LoadMaterialTextures(material, aiTextureType_AMBIENT);
		//for (auto texture : textureList)
		//{
		//	std::weak_ptr foundModel = Texture_Flyweight::FindTexture(texture, m_device, m_deviceCon);
		//	std::shared_ptr<TextureClass> foundModelShared = foundModel.lock();

		//	if (foundModelShared)
		//	{
		//		textures.push_back(foundModelShared.get());
		//	}

		//}
	}

	return Mesh(m_device, vertices, indices, textures);
}

bool ModelComponent::LoadModel(char* filename)
{
	Assimp::Importer importer;

	// TODO - Add check for model origin eg from blender or from 3DS Max.
	const aiScene* pScene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (pScene == nullptr)
	{
		// TODO: Convert this to a messagebox.
		std::string string = importer.GetErrorString();

		return false;
	}

	ProcessNode(pScene->mRootNode,pScene);

	return true;
}

std::vector<TextureClass> ModelComponent::LoadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
	std::vector<TextureClass> textures;
	bool result;

	aiString pathStr;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		mat->GetTexture(type, i, &pathStr);

		weak_ptr<TextureClass> weakTexturePtr = Texture_Flyweight::FindTexture(pathStr.C_Str(), m_device, m_deviceCon);
		shared_ptr<TextureClass> sharedTexturePtr = weakTexturePtr.lock();

		if (sharedTexturePtr)
		{
			textures.push_back(*sharedTexturePtr.get());
		}

	}
	return textures;
}