#ifndef MODEL_COMPONENT_H
#define MODEL_COMPONENT_H

#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "ConstantBuffer.h"
#include "TextureFlyweight.h"

class TextureClass;
class Mesh;
struct VertexType;

class ModelComponent
{
	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

public:
	ModelComponent();
	ModelComponent(const ModelComponent&);
	~ModelComponent();

	/*
	* \brief Loads the model data by path into m_model, initialises the vertex and index buffers, and loads the texture if a mtl is present.
	* \returns True if done successfully. Returns false if any part of initialisation fails.
	*/
	bool Initialise(ID3D11Device* device, ID3D11DeviceContext* deviceCon, char* modelPath);

	// \brief Releases the texture data, buffers, and model data.
	void Shutdown();

	// \brief Renders the textured model to the screen.
	void Render(ID3D11DeviceContext* deviceCon, MatrixBuffer& mb);

	ID3D11ShaderResourceView* GetTexture();

	int GetIndexCount();

	// \brief Reads the model data from path using assimp and populates m_model vector with the vertex data.
	bool LoadModel(char* path);

private:
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<TextureClass> LoadMaterialTextures(aiMaterial* mat, aiTextureType type);

	std::vector<Mesh> m_meshes;

	UINT m_indexCount;

	// TODO: Convert to smartptr
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceCon = nullptr;

	MatrixBuffer m_cbData;
	ID3D11Buffer* m_constantBuffer;

	bool m_initialised = false;
};
#endif

