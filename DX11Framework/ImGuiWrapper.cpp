#include "ImGuiWrapper.h"

// ImGui
#include <imgui_internal.h>
#include <backend/imgui_impl_win32.h>
#include <backend/imgui_impl_dx11.h>

#include "GameObject.h"
#include "TransformComponent.h"

using namespace DirectX;

bool ImGuiWrapper::m_initalised = false;

ImGuiWrapper::ImGuiWrapper() {}

ImGuiWrapper::~ImGuiWrapper(){}

void ImGuiWrapper::Initialise(HWND hwnd, ID3D11Device* device, ID3D11DeviceContext* deviceCon)
{
	IMGUI_CHECKVERSION();
	// Make process DPI aware and obtain main monitor scale
	ImGui_ImplWin32_EnableDpiAwareness();
	float main_scale = ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd);

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;      // Enable Docking

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight(); // Maybe I should implement switchable colour palettes.

	// Setup scaling
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
	style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

	m_viewportTexture = new RenderTextureClass();
	m_viewportTexture->Initialise(device, 1280, 768, 0.3f, 1000.0f, 1);

	m_blurring = new Blurring(device, deviceCon, hwnd);

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(device, deviceCon);

	m_initalised = true;
}

void ImGuiWrapper::Shutdown()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();
}

void ImGuiWrapper::DockSpace()
{
	constexpr bool open = false;
	constexpr bool opt_fullscreen = true;
	constexpr ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	static bool dockspaceBuilt = false;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();

		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		window_flags |=  ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar;
	}
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(3);

	// DockSpace
	ImGuiID dockspaceID = ImGui::GetID("DockSpace");
	ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspace_flags);

	if (!dockspaceBuilt)
	{
		dockspaceBuilt = true;

		ImGui::DockBuilderRemoveNode(dockspaceID);             // Clear any existing layout.
		ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace); // Add a new node.

		// Split the dockspace into 3 parts.
		ImGuiID dockMainId = dockspaceID;
		ImGuiID dockLeftID = ImGui::DockBuilderSplitNode(dockMainId, ImGuiDir_Left, 0.5f, nullptr, &dockMainId);
		ImGui::DockBuilderSetNodeSize(dockLeftID, ImVec2{300, 600});
		ImGuiID dockIDNewObj = ImGui::DockBuilderSplitNode(dockLeftID, ImGuiDir_Down, 0.3f, nullptr, &dockLeftID);

		// Dock windows
		ImGui::DockBuilderDockWindow("Viewport", dockMainId);
		ImGui::DockBuilderDockWindow("Transform", dockLeftID);
		ImGui::DockBuilderDockWindow("New", dockIDNewObj);

		ImGui::DockBuilderFinish(dockspaceID);
	}

	ImGui::End();
}

void ImGuiWrapper::Update(float deltaTime)
{
	ImGui_ImplWin32_NewFrame();
	ImGui_ImplDX11_NewFrame();
	ImGui::NewFrame();

	DockSpace();
}

void ImGuiWrapper::Render()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGui::UpdatePlatformWindows();
	ImGui::EndFrame();
}

void ImGuiWrapper::TransformPanel(GameObject& obj)
{
	ImGui::Begin("Transform");

	int index = 0;
	constexpr int resetToZero = 0;
	constexpr int resetToOne = 1;

	// Position
	DrawVec3Control(obj.m_transform->m_position, "Position", index, resetToZero);

	// Rotation
	index++;
	DrawVec3Control(obj.m_transform->m_rotation, "Rotation", index, resetToZero);

	// Scale
	index++;
	DrawVec3Control(obj.m_transform->m_scale, "Scale", index, resetToOne);

	ImGui::End();
}

void ImGuiWrapper::ViewportStart(ID3D11DeviceContext* deviceCon)
{
	m_viewportTexture->ClearRenderTarget(deviceCon, 0.025f, 0.025f, 0.025f, 1.0f);
	m_viewportTexture->SetRenderTarget(deviceCon);
}

void ImGuiWrapper::ViewportUpdate(ID3D11DeviceContext* deviceCon)
{
	ImGui::Begin("Viewport");

	// Display Viewport.
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	ImVec2 size = { viewportPanelSize.x, viewportPanelSize.y };

	ImGui::Image((ImTextureID)(intptr_t)m_viewportTexture->GetShaderResourceView(), ImVec2{ size.x, size.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });

	ImGui::End();
}


void ImGuiWrapper::BlurredViewportStart(ID3D11Device* device, ID3D11DeviceContext* deviceCon)
{
	m_blurring->CopyViewport(device, deviceCon, m_viewportTexture->GetShaderResourceView());
}

void ImGuiWrapper::GaussianBlur(ID3D11DeviceContext* devCon)
{
	m_blurring->Gaussian(devCon);
}

void ImGuiWrapper::BlurredViewportUpdate(ID3D11DeviceContext* deviceCon)
{
	ImGui::Begin("Blurred Viewport");

	// Display Viewport.
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	ImVec2 size = { viewportPanelSize.x, viewportPanelSize.y };

	ImGui::Image((ImTextureID)(intptr_t)m_blurring->GetShaderResourceView(), ImVec2{ size.x, size.y }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 });

	ImGui::End();
}

void ImGuiWrapper::WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
	if (m_initalised)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_WndProcHandler(hwnd, umessage, wparam, lparam);
	}
}

int ImGuiWrapper::NewObjectPanel()
{
	int returnItem = -1;

	ImGui::Begin("New");
	ImGui::Combo("Object Type", &m_selectedItem, m_shapeList, IM_ARRAYSIZE(m_shapeList));

	if (ImGui::Button("New Obj"))
	{
		returnItem = m_selectedItem;
	}
	ImGui::End();

	return returnItem;
}

bool ImGuiWrapper::SaveChanges()
{
	bool isPressed = false;

	ImGui::Begin("Save");

	if (ImGui::Button("Save Changes"))
	{
		isPressed = true;
	}

	ImGui::End();

	return isPressed;
}

void ImGuiWrapper::LightPanel(float* ambientCol, float* diffuseCol,
	float* specularCol, float* specularPower,
	XMFLOAT4& lightDir)
{
	constexpr int resetToZero = 0;

	constexpr float minSpecPower = 1.0f; 
	constexpr float maxSpecPower = 64.0f; 

	ImGui::Begin("Edit Light");

	ImGui::ColorEdit4("Ambient Colour", ambientCol);

	ImGui::NewLine();

	ImGui::ColorEdit4("Diffuse Colour", diffuseCol);

	ImGui::NewLine();

	ImGui::ColorEdit4("Specular Colour", specularCol);
	ImGui::SliderFloat("Specular Power", specularPower , minSpecPower, maxSpecPower, "%.1f");

	ImGui::NewLine();

	XMFLOAT3 lightDir3 = XMFLOAT3(lightDir.x, lightDir.y, lightDir.z);
	DrawVec3Control(lightDir3, "Direction", 4, resetToZero);
	lightDir.x = lightDir3.x; lightDir.y = lightDir3.y; lightDir.z = lightDir3.z;


	ImGui::End();
}

void ImGuiWrapper::GaussDataPanel()
{
	constexpr int intSliderSpeed = 1;
	constexpr int minKernel = 1;
	constexpr int maxKernel = 10;

	constexpr float floatSliderSpeed = 0.1f;
	constexpr float minSamplePos = 0.0f;
	constexpr float maxSamplePos = 100.0f;

	constexpr float minSigma = 0.1f;
	constexpr float maxSigma = 10.0f; // Literally me.

	ImGui::Begin("Edit Gaussian Data");

	ImGui::DragInt("Kernel Size", &m_blurring->m_gaussData.KernelSize, intSliderSpeed, minKernel, maxKernel);
	ImGui::DragFloat("Sample Pos Multiplier", &m_blurring->m_gaussData.SamplePosMultiplier, floatSliderSpeed, minSamplePos, maxSamplePos);
	ImGui::DragFloat("Sigma", &m_blurring->m_gaussData.Sigma, floatSliderSpeed, minSigma, maxSigma);


	ImGui::End();
}

void ImGuiWrapper::DrawVec3Control(XMFLOAT3& vector, std::string displayString, int index,float resetTo, float columnWidth, float barWidth)
{
	std::string id = "##Table" + index;

	bool tableCreated = ImGui::BeginTable(id.c_str(), 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingFixedFit);

	// Program can crash if table is not verified.
	if (!tableCreated) return;

	ImGui::TableSetupColumn("Left", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, columnWidth);
	ImGui::TableSetupColumn("Right", ImGuiTableColumnFlags_None);

	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);

	ImGui::Text(displayString.c_str());

	ImGui::TableSetColumnIndex(1);

	ImGui::PushItemWidth(barWidth);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });

	float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	id = "X##" + std::to_string(index);

	if (ImGui::Button(id.c_str()))
	{
		vector.x = resetTo;
	}

	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	id = "##X" + std::to_string(index);
	ImGui::DragFloat(id.c_str(), &vector.x, 0.2f, 0.0f, 0.0f, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushItemWidth(barWidth);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });

	id = "Y##" + std::to_string(index);
	if (ImGui::Button(id.c_str()))
	{
		vector.y = resetTo;
	}

	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	id = "##Y" + std::to_string(index);
	ImGui::DragFloat(id.c_str(), &vector.y, 0.2f, 0.0f, 0.0f, "%.3f");
	ImGui::PopItemWidth();
	ImGui::SameLine();


	ImGui::PushItemWidth(barWidth);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });

	id = "Z##" + std::to_string(index);
	if (ImGui::Button(id.c_str()))
	{
		vector.z = resetTo;
	}

	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	id = "##Z" + std::to_string(index);
	ImGui::DragFloat(id.c_str(), &vector.z, 0.2f, 0.0f, 0.0f, "%.3f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::EndTable();
}