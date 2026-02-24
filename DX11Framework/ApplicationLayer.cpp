#include "ApplicationLayer.h"

#include "TextureClass.h"
#include <fstream>
#include <algorithm>
#include "ModelComponent.h"
#include "ImGuiWrapper.h"
#include "DX11Framework.h"
#include "TransformComponent.h"
#include "DebugCamera.h"
#include "DollyCamera.h"

#include "DirectionalLight.h"
#include "PointLight.h"

#include "SkyBox.h"
#include "Input.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace DirectX;
using namespace std;
using Microsoft::WRL::ComPtr;

bool ApplicationLayer::Initialise(ID3D11Device* device, ID3D11DeviceContext* devCon, HWND windowHandle)
{
    m_DevicePtr = device;
    m_DevConPtr = devCon;
    m_WindowHandle = windowHandle;

    m_ui = new ImGuiWrapper();
    m_ui->Initialise(windowHandle, device, devCon);

    m_model = new ModelComponent;
    m_model->Initialise(device, devCon, (char*)"Models/Iggy/model.obj");

    m_skybox = new SkyBox();
    const char* skyboxTexturePaths[6] = {
        "Skybox/pz.png", // right
        "Skybox/nz.png", // Left
        "Skybox/py.png", // Top
        "Skybox/ny.png", // Bottom.
        "Skybox/nx.png", // Back
        "Skybox/px.png", // Front
    };

    m_skybox->Initialise(device, devCon, windowHandle, skyboxTexturePaths);

    LoadScene("CameraSaveTest.json");

    m_pointLight = new PointLight({ 1.0f, 1.0f, 1.0f,1.0f }, { 0.1f, 0.1f, 0.1f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }, 10,
        { 0.0f, 0.0f, 3.0f });
    m_pointLight->Initialise(device);

    m_pointLight2 = new PointLight({ 1.0f, 1.0f, 1.0f,1.0f }, { 0.1f, 0.1f, 0.1f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }, 10,
        { 0.0f, 2.0f, -3.0f });
    m_pointLight2->Initialise(device);

    m_dirLight = new DirectionalLight({ 1.0f, 1.0f, 1.0f,1.0f }, { 0.1f, 0.1f, 0.1f, 1.0f },
        { 1.0f, 1.0f, 1.0f, 1.0f }, 10,
        { 0.0f, 1.0f, -1.0f });
    m_dirLight->Initialise(device);

    return true;
}

void ApplicationLayer::Update(float deltaTime)
{
    m_camVector[m_activeCamera]->Update(deltaTime);

    m_ui->Update(deltaTime);

    switch (m_ui->NewObjectPanel())
    {
    case m_ui->Primitives::Cube:
        CreateNewObject((char*)"Primitives/Prim_Cube.obj", (char*)"stone01.tga");
        break;

    case m_ui->Primitives::Plane:
        CreateNewObject((char*)"Primitives/Prim_Plane.obj", (char*)"stone01.tga");
        break;

    case m_ui->Primitives::Sphere:
        CreateNewObject((char*)"Primitives/Prim_Sphere.obj", (char*)"stone01.tga");
        break;

    case m_ui->Primitives::Cylinder:
        CreateNewObject((char*)"Primitives/Prim_Cylinder.obj", (char*)"stone01.tga");
        break;

    case m_ui->Primitives::Cone:
        CreateNewObject((char*)"Primitives/Prim_Cone.obj", (char*)"stone01.tga");
        break;

    case m_ui->Primitives::Torus:
        CreateNewObject((char*)"Primitives/Prim_Torus.obj", (char*)"stone01.tga");
        break;

    case m_ui->Primitives::Dragon:
        CreateNewObject((char*)"Primitives/dragon.obj", (char*)"stone01.tga");
        break;
    }

    m_ui->TransformPanel(*m_objVector[m_selectedObj]);

    m_ui->LightPanel(m_pointLight->GetAmbientColour(),
        m_pointLight->GetDiffuseColour(),
        m_pointLight->GetSpecularColour(), m_pointLight->GetSpecularPower(),
        m_pointLight->GetLightPos());

    m_ui->GaussDataPanel();

    if (Input::GetKeyDown('1'))
    {
        // Increment current cam.
        m_activeCamera++;

        // Wrap around vector.
        if (m_activeCamera >= m_camVector.size())
        {
            m_activeCamera = 0;
        }
    }

    if (Input::GetKeyDown('2'))
    {
        // Increment current cam.
        m_selectedObj++;

        // Wrap around vector.
        if (m_selectedObj >= m_objVector.size())
        {
            m_selectedObj = 0;
        }
    }

    if (m_ui->SaveChanges())
    {
        SaveScene("CameraSaveTest.json");
    }
}

void ApplicationLayer::Draw(MatrixBuffer& mb, DX11Framework* dx)
{
    m_ui->ViewportStart(m_DevConPtr.Get());

    mb.World = XMMatrixTranspose(XMMatrixIdentity());
    mb.View = XMMatrixTranspose(XMLoadFloat4x4(&m_camVector[m_activeCamera]->GetView()));
    mb.Projection = XMMatrixTranspose(XMLoadFloat4x4(&m_camVector[m_activeCamera]->GetProj()));

    m_pointLight->Render(m_DevConPtr.Get());

    for (int i = 0; i < m_objVector.size(); i++)
    {
        // Ensure rendered object isn't a child to prevent chldren being rendered twice.
        if (!m_objVector[i]->IsChild())
            m_objVector[i]->Render(m_DevConPtr.Get(), mb);
    }

    m_skybox->Render(m_DevConPtr.Get(), mb, m_camVector[m_activeCamera]->GetEye());

    m_ui->ViewportUpdate(m_DevConPtr.Get());

    //m_ui->BlurredViewportStart(m_DevicePtr.Get(), m_DevConPtr.Get());
    //m_ui->GaussianBlur(m_DevConPtr.Get());
    //m_ui->BlurredViewportUpdate(m_DevConPtr.Get());

    dx->SetBackBufferRenderTarget();
    dx->ResetViewport();

    m_ui->Render();
}

void ApplicationLayer::UIMessage(HWND windowHandle, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    m_ui->WndProc(windowHandle, umessage, wparam, lparam);
}

void ApplicationLayer::CreateNewObject(char* modelPath, char* texturePath)
{
    m_objVector.emplace_back(std::make_shared<GameObject>(m_DevicePtr.Get(), m_WindowHandle));
    bool result = m_objVector.back()->LoadModel(m_DevicePtr.Get(), m_DevConPtr.Get(), modelPath, texturePath);
}

bool ApplicationLayer::LoadScene(string path)
{
	auto ReadXMFloat3 = [](const string name, const json& desc)
	{
		json jsonVector = desc[name];

		float x = jsonVector["X"], y = jsonVector["Y"], z = jsonVector["Z"];

		return XMFLOAT3(x, y, z);
	};

	json jFile;
	std::ifstream fileOpen(path);
	if (fileOpen.fail())
	{
		return false;
	}

	jFile = json::parse(fileOpen);

	std::string SceneName = jFile["SceneName"].get<std::string>();
	m_activeCamera = jFile["Active Cam"].get<int>();

	json& objects = jFile["GameObjects"];

	int size = objects.size();
	m_objVector.reserve(size);
	for (int i = 0; i < size; i++)
	{
		//Vector3 translation, rotation, scale;

		json& objectDesc = objects.at(i);
		std::string modelPath = objectDesc["ModelPath"];
		std::string objName = objectDesc["Name"];

		m_objVector.emplace_back(std::make_shared<GameObject>(m_DevicePtr.Get(), m_WindowHandle));
		m_objVector.back().get()->LoadModel(m_DevicePtr.Get(), m_DevConPtr.Get(), (char*)modelPath.c_str(), (char*)"stone01.tga");
		m_objVector[i]->m_name = objName;

		TransformComponent* transform = m_objVector.back()->m_transform;
		transform->m_position = ReadXMFloat3("Position", objectDesc);
		transform->m_rotation = (ReadXMFloat3("Rotation", objectDesc));
		transform->m_scale = ReadXMFloat3("Scale", objectDesc);
	}

	json& cams = jFile["Cameras"];
	size = cams.size();
	m_camVector.reserve(size);
	for (int i = 0; i < size; i++)
	{
		json& camDesc = cams.at(i);

		std::string camType = camDesc["Camera Type"];

		XMFLOAT3 eye = ReadXMFloat3("Eye", camDesc);
		XMFLOAT3 at = ReadXMFloat3("At", camDesc);
		XMFLOAT3 up = ReadXMFloat3("Up", camDesc);

		if (camType == "Base")
		{
			m_camVector.push_back(std::make_shared<BaseCamera>(eye, at, up,
				1280, 768, 0.01f, 100.0f));
		}
		else if (camType == "Debug")
		{
			float yaw = camDesc["Yaw"];
			float pitch = camDesc["Pitch"];
			float sensitivity = camDesc["Sensitivity"];
			float speed = camDesc["Speed"];

			std::shared_ptr<DebugCamera> debugPtr = std::make_shared<DebugCamera>(eye, at, up,
				1280, 768, 0.01f, 100.0f);
			debugPtr->Initialise(yaw, pitch, sensitivity, speed);

			m_camVector.push_back(debugPtr);
		}
	}
	return true;
}

bool ApplicationLayer::SaveScene(string path)
{
	auto SaveXMFloat3 = [](const string name, const XMFLOAT3& vector)
		{
			return json(
				{
					name, {
					{"X", vector.x },
					{"Y", vector.y },
					{"Z", vector.z },
					}
				});
		};

	std::ofstream fileWrite(path);

	json gameObjectsJson = json::array();

	for (int i = 0; i < m_objVector.size(); i++)
	{
		std::string modelPath = m_objVector[i]->m_modelPath;
		std::string name = m_objVector[i]->m_name;

		TransformComponent* transform = m_objVector[i]->m_transform;
		
		gameObjectsJson.push_back(json::object(
			{
					{"ModelPath", modelPath},
					// {"BaseColourPath", baseColourPath},

					{"Name", name},

					SaveXMFloat3("Position", transform->m_position),
					SaveXMFloat3("Rotation", transform->m_rotation),
					SaveXMFloat3("Scale", transform->m_scale),
			}));
	}

	json camerasJson = json::array();
	for (std::shared_ptr<BaseCamera> cam : m_camVector)
	{
		std::string camType = cam->GetType();


		if (camType == "Base")
		{
			camerasJson.push_back(json::object(
				{
					{"Camera Type", camType},

					SaveXMFloat3("Eye", cam->GetEye()),
					SaveXMFloat3("At", cam->GetAt()),
					SaveXMFloat3("Up", cam->GetUp()),
				}));
		}

		else if (camType == "Debug")
		{
			DebugCamera* debugCam = (DebugCamera*)cam.get();

			float yaw = debugCam->GetYaw();
			float pitch = debugCam->GetPitch();
			float speed = debugCam->m_speed;
			float sensitivty = debugCam->m_sensitivity;

			camerasJson.push_back(json::object(
				{
					{"Camera Type", camType},

					SaveXMFloat3("Eye", cam->GetEye()),
					SaveXMFloat3("At", cam->GetAt()),
					SaveXMFloat3("Up", cam->GetUp()),

					{"Yaw", yaw},
					{"Pitch", pitch},

					{"Speed", speed},
					{"Sensitivity", sensitivty}
				}));
		}

		fileWrite << json::object({
			{"GameObjects", gameObjectsJson},
			{"Cameras", camerasJson},
			{"Active Cam", m_activeCamera},
			{"SceneName", "Osaka"},
			}).dump(2) << std::endl;

		return true;
	}
}