#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <d3d11.h>
#include <directxmath.h>

#include "ConstantBuffer.h"
#include "ImGuiWrapper.h"

#include <list>
#include <memory>

class TransformComponent;
class ModelComponent;

class GameObject
{
public:
	GameObject(ID3D11Device* device, HWND windowHandle);
	~GameObject();

	void Update(float deltaTime);
	void Render(ID3D11DeviceContext* deviceCon, MatrixBuffer& mb);
	void Release();

	bool LoadModel(ID3D11Device* device, ID3D11DeviceContext* deviceCon, char* modelPath);
	std::string GetModelPath();

	TransformComponent* GetTransform();

	inline bool IsChild() { return m_parent != nullptr; }

public:
	std::string m_name = "Default Object";

	std::string m_modelPath = "";

	void AddChild(GameObject* goPtr);
	void RemoveChild(GameObject* goPtr);

	bool m_isSelected = false;
	std::list<GameObject*> m_children; // Doesn't need to be contiguous.

	static int m_nextID;
	int m_id = 0;
private:
	void TransformSelfAndChildren();

private:
	GameObject* m_parent = nullptr;

	std::unique_ptr<ModelComponent> m_model = nullptr;
	std::unique_ptr<TransformComponent> m_transform = nullptr;

	bool m_initialised = false;

	friend ImGuiWrapper;
};

#endif