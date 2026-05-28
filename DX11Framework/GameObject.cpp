#include "GameObject.h"

#include "TransformComponent.h"
#include "ModelComponent.h"
#include "HelperMacros.h"

using namespace DirectX;
using namespace std;

// TODO: rework selection.
int GameObject::m_nextID = -1;

GameObject::GameObject(ID3D11Device* device, HWND windowHandle)
{
	m_transform = std::make_unique<TransformComponent>(XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f),
		XMFLOAT3(0.0f, 0.0f, 0.0f));

	if (m_nextID != -1)
	{
		m_id = m_nextID;
		m_nextID++;
	}

	m_initialised = true;
}

GameObject::~GameObject()
{
	if (m_model) m_model.release();
	if (m_transform) m_transform.release();
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

void GameObject::Release()
{
	if (m_model) m_model.release();
	if (m_transform) m_transform.release();
}

bool GameObject::LoadModel(ID3D11Device* device, ID3D11DeviceContext* deviceCon, char* modelPath)
{
	bool result;
	if (!m_model)
		m_model = std::make_unique<ModelComponent>();

	result = m_model->Initialise(device, deviceCon, modelPath);
	if (!result)
	{
		return false;
	}

	m_modelPath = modelPath;
	return true;
}

std::string GameObject::GetModelPath() { return m_modelPath; }

TransformComponent* GameObject::GetTransform() { return m_transform.get(); }

void GameObject::AddChild(GameObject* goPtr)
{
	m_children.emplace_back(goPtr);
	m_children.back()->m_parent = this; // Set childs parent.
}

void GameObject::RemoveChild(GameObject* goPtr)
{
	if (m_children.empty()) return;

	list<GameObject*>::iterator iter;
	for (iter = m_children.begin(); iter != m_children.end(); iter++)
	{
		GameObject* iterObj = *iter;
		if (iterObj->m_name == goPtr->m_name)
		{
			m_children.erase(iter);
			return;
		}
	}
}

void GameObject::TransformSelfAndChildren()
{

}
