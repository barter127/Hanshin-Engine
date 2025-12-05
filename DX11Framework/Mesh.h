#ifndef MESH_CLASS_H_
#define MESH_CLASS_H_

#include <vector>

#include <d3d11.h>
#include <directxmath.h>

#include "ConstantBuffer.h"

class TextureClass;

struct VertexType
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texcoord;
	DirectX::XMFLOAT3 normal;
};

class Mesh
{
public:

public:
	Mesh();
	Mesh(ID3D11Device* device, std::vector<VertexType> vertices, std::vector<UINT> indices, std::vector<TextureClass> textures);
	~Mesh();

	void Initialise(ID3D11Device* device, std::vector<VertexType> vertices, std::vector<UINT> indices);
	void Draw(ID3D11DeviceContext* deviceCon);
	void Release();

private:
	bool InitBuffers(ID3D11Device* device);

private:
    std::vector<VertexType> m_vertices;
    std::vector<UINT> m_indices;
    std::vector<TextureClass>  m_textures;

	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;

	ID3D11SamplerState* m_bilinearSamplerState;
};

#endif //MESH_CLASS_H_