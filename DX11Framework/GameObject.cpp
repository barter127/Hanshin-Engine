#include "GameObject.h"

#include "TransformComponent.h"
#include "ModelComponent.h"

using namespace DirectX;

GameObject::GameObject(ID3D11Device* device, HWND windowHandle)
{
	m_transform = new TransformComponent({ 0.0f, 0.0f, 0.0f },
		{ 0.0f,0.0f,0.0f },
		{ 1.0f,1.0f,1.0f });

	m_initialised = true;
}

GameObject::~GameObject()
{
	if (m_model) delete m_model;
	if (m_transform) delete m_transform;
}

bool GameObject::LoadModel(ID3D11Device* device, ID3D11DeviceContext* deviceCon, char* modelPath, char* texturePath)
{
	bool result;
	if (m_model) delete m_model;
	m_model = new ModelComponent();

	result = m_model->Initialise(device, deviceCon, modelPath);
	if (!result)
	{
		return false;
	}

	m_modelPath = modelPath;
	m_texturePath = texturePath;

	return true;
}

std::string GameObject::GetModelPath()
{
	return m_modelPath;
}

std::string GameObject::GetTexturePath()
{
	return m_texturePath;
}

void GameObject::Update(float deltaTime)
{
	// Nothing rn lol.
}

void GameObject::Render(ID3D11DeviceContext* deviceCon, MatrixBuffer& mb)
{
	if (m_initialised)
	{
		mb.World *= XMMatrixTranspose(m_transform->GetTransformMatrix());
		m_model->Render(deviceCon, mb);

		for (auto&& child : m_children)
		{
			child->Render(deviceCon, mb);
		}

		mb.World *= XMMatrixTranspose(m_transform->GetInverseMatrix());
	}
}

void GameObject::AddChild(GameObject* goPtr)
{
	m_children.emplace_back(goPtr);
	m_children.back()->m_parent = this; // Set childs parent.
}

void GameObject::TransformSelfAndChildren()
{

}
