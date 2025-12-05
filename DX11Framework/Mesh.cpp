#include "Mesh.h"

#include "TextureClass.h"

using namespace DirectX;

Mesh::Mesh() {}

Mesh::Mesh(ID3D11Device* device, std::vector<VertexType> vertices, std::vector<UINT> indices, std::vector<TextureClass> textures) :
	m_vertices(vertices),
	m_indices(indices),
	m_textures(textures)
{
	Initialise(device, vertices, indices);
}

Mesh::~Mesh() 
{
	// Release();
}

void Mesh::Initialise(ID3D11Device* device, std::vector<VertexType> vertices, std::vector<UINT> indices)
{
	m_vertices = vertices;
	m_indices = indices;

	InitBuffers(device);

	// Maybe this could be moved to modelclass.
	D3D11_SAMPLER_DESC bilinearSamplerDesc = {};
	bilinearSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	bilinearSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	bilinearSamplerDesc.MaxLOD = 1;
	bilinearSamplerDesc.MinLOD = 0;

	device->CreateSamplerState(&bilinearSamplerDesc, &m_bilinearSamplerState);
}

void Mesh::Draw(ID3D11DeviceContext* deviceCon)
{
	//Set object variables and draw
	UINT stride = { sizeof(VertexType) };
	UINT offset = 0;
	deviceCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceCon->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceCon->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0); // tells index buffer uses WORDs (2 byte for each index)

	deviceCon->PSSetSamplers(0, 1, &m_bilinearSamplerState); // Maybe I could have a UI element to change samplers.

	if (m_textures.size() > 0)
	{
		deviceCon->PSSetShaderResources(0, 1, &m_textures[0].m_textureView);
		deviceCon->PSSetShaderResources(1, 1, &m_textures[1].m_textureView);
		deviceCon->PSSetShaderResources(2, 1, &m_textures[2].m_textureView);
		deviceCon->PSSetShaderResources(3, 1, &m_textures[3].m_textureView);
	}

	deviceCon->DrawIndexed(m_indices.size(), 0, 0);
}

void Mesh::Release()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

bool Mesh::InitBuffers(ID3D11Device* device)
{
	Release();

	VertexType* vertices;
	UINT* indices;
	HRESULT hr;

	// Create the vertex array.
	vertices = new VertexType[m_vertices.size()];

	// Load the vertex array with data.
	for (int i = 0; i < m_vertices.size(); i++)
	{
		vertices[i] = m_vertices[i];
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertices.size() ;
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER; // type of buffer

	D3D11_SUBRESOURCE_DATA vertexData = { vertices };

	hr = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hr)) return hr;

	// Create the index array.
	indices = new UINT[m_indices.size()];

	// Load the index array with data.
	for (int i = 0; i < m_indices.size(); i++)
	{
		indices[i] = m_indices[i];
	}

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = sizeof(UINT) * m_indices.size();
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = { indices };

	hr = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hr)) return hr;

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}