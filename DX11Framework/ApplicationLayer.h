#ifndef APPLICATION_LAYER_H
#define APPLICATION_LAYER_H

#include <string>
#include <vector>
#include <memory>

#include <DirectXMath.h>
#include <wrl.h>

#include "GameObject.h"
#include "BaseCamera.h"
#include "ConstantBuffer.h"

class ImGuiWrapper;
class DX11Framework;
class DirectionalLight;
class PointLight;
class SkyBox;

class ApplicationLayer
{
public:
	bool Initialise(ID3D11Device* device, ID3D11DeviceContext* devCon, HWND windowHandle);
	void Update(float deltaTime);
	void Draw(MatrixBuffer& mb, DX11Framework* dx);

	void UIMessage(HWND windowHandle, UINT umessage, WPARAM wparam, LPARAM lparam);

private:
	void CreateNewObject(char* modelPath, char* texturePath);
	bool ReadJson(std::string path);
	bool WriteJson(std::string path);

	ImGuiWrapper* m_ui;

	ModelComponent* m_model;

	int m_selectedObj = 0;
	std::vector<std::shared_ptr<GameObject>>m_objVector;

	int m_activeCamera = 2;
	std::vector<std::shared_ptr<BaseCamera>>m_camVector;

	SkyBox* m_skybox = nullptr;
	PointLight* m_pointLight = nullptr;
	PointLight* m_pointLight2 = nullptr;
	DirectionalLight* m_dirLight = nullptr;

	UINT _lightCount;

	Microsoft::WRL::ComPtr<ID3D11Device> m_DevicePtr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DevConPtr;
	HWND m_WindowHandle;
};

#endif
