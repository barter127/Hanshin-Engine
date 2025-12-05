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

	bool LoadModel(ID3D11Device* device, ID3D11DeviceContext* deviceCon, char* modelPath, char* texturePath);
	std::string GetModelPath();
	std::string GetTexturePath();

	void Update(float deltaTime);
	void Render(ID3D11DeviceContext* deviceCon, MatrixBuffer& mb);

	inline bool IsChild() { return m_parent != nullptr; }

public:
	TransformComponent* m_transform = nullptr;
	std::string m_name = "Default Object";

	std::string m_modelPath = "";
	std::string m_texturePath = "";

	void AddChild(GameObject* goPtr);

private:
	void TransformSelfAndChildren();

private:
	GameObject* m_parent = nullptr;
	std::list<std::shared_ptr<GameObject>> m_children; // Doesn't need to be contiguous.

	ModelComponent* m_model = nullptr;

	bool m_initialised = false;

	friend ImGuiWrapper;
};

#endif