#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <imgui.h>

#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl.h>

#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <stack>

#include "RenderTextureClass.h"
#include "Blurring.h"

class GameObject;
class TextureClass;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class ImGuiWrapper
{
public:
	ImGuiWrapper();
	~ImGuiWrapper();

	void Initialise(HWND hwnd, ID3D11Device* device,  ID3D11DeviceContext* deviceCon);
	void Shutdown();

	void Update(float deltaTime);
	void Render();

	void TransformPanel(GameObject& obj);
	int NewObjectPanel();
	void LightPanel(float* ambientCol, float* diffuseCol,
		float* specularCol, float* specularPower,
		DirectX::XMFLOAT4& lightDir);
	void GaussDataPanel();

	void SceneGraph(std::vector<std::shared_ptr<GameObject>>& objVector);
	bool CreateSceneNode(GameObject* object);
	void AcceptLoad(GameObject* object);
	void AcceptLoadRoot();

	// Content Browser.
	std::stack<std::string> m_pathStack; // ordinarily I'd put this at the bottom but as I'm gonna refactor it's here.
	std::string GetFileName(std::filesystem::directory_entry entry, std::string path);
	void DisplayTexture(std::filesystem::directory_entry entry, std::string displayName);
	bool DisplayFolder(std::filesystem::directory_entry entry, std::string displayName);
	void EnterFolder(std::filesystem::directory_entry entry);
	void ExitCurrentFolder();
	void ContentBrowser();


	bool SaveChanges();

	void ViewportStart(ID3D11DeviceContext* deviceCon);
	void ViewportUpdate(ID3D11DeviceContext* deviceCon);
	void BlurredViewportStart(ID3D11Device* device, ID3D11DeviceContext* deviceCon);
	void GaussianBlur(ID3D11DeviceContext* devCon);
	void BlurredViewportUpdate(ID3D11DeviceContext* deviceCon);

	void DockSpace();

	// For simplicity just allowing 1 object to be selected at a time.
	int GetSelectedIndex() { return m_selectedItem; }

public:
	enum Primitives
	{
		Cube = 0,
		Plane,
		Sphere,
		Cylinder,
		Cone,
		Torus,
		Dragon
	};

	// Maybe I could turn this into an array/vector.
	RenderTextureClass* m_viewportTexture = nullptr;
	Blurring* m_blurring = nullptr; // Maybe move this.

private:
	void DrawVec3Control(DirectX::XMFLOAT3& vector, std::string displayString, int index, float buttonVal = 0.0f, float columnWidth = 75.0f, float barWidth = 50.0f);

private:
	static bool m_initalised;

	int m_selectedItem = 0;
	const char* m_shapeList[7] = { "Cube", "Plane","Sphere", "Cylinder", "Cone", "Torus", "Dragon" };

	TextureClass* m_folderTexture = nullptr;

	Microsoft::WRL::ComPtr<ID3D11Device> m_DevicePtr;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_DevConPtr;
	HWND m_WindowHandle;
};

#endif // !IMGUI_WRAPPER_H