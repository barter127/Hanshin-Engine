#ifndef IMGUI_WRAPPER_H
#define IMGUI_WRAPPER_H

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <imgui.h>

#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <string>

#include "RenderTextureClass.h"
#include "Blurring.h"

class GameObject;

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

	bool SaveChanges();

	void ViewportStart(ID3D11DeviceContext* deviceCon);
	void ViewportUpdate(ID3D11DeviceContext* deviceCon);
	void BlurredViewportStart(ID3D11Device* device, ID3D11DeviceContext* deviceCon);
	void GaussianBlur(ID3D11DeviceContext* devCon);
	void BlurredViewportUpdate(ID3D11DeviceContext* deviceCon);

	void WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam);

	void DockSpace();


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
};

#endif // !IMGUI_WRAPPER_H