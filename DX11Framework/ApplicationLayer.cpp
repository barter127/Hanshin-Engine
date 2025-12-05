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

    ReadJson("CameraSaveTest.json");

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
        

    // Child to Iggy and the Fool.
    m_objVector.emplace_back(std::make_shared<GameObject>(m_DevicePtr.Get(), m_WindowHandle));
    m_objVector.back()->LoadModel(device, devCon, (char*)"Primitives/Prim_Cube.obj", (char*)"stone01.tga");
    m_objVector.back()->m_transform->m_position = { 2, 1, -10 };
    m_objVector[0]->AddChild(m_objVector.back().get());

    // Child to just created cube
    m_objVector.emplace_back(std::make_shared<GameObject>(m_DevicePtr.Get(), m_WindowHandle));
    m_objVector.back()->LoadModel(device, devCon, (char*)"Primitives/Prim_Torus.obj", (char*)"stone01.tga");
    m_objVector.back()->m_transform->m_position = { 2, 1, -10 };
    int lastObjectIndex = m_objVector.size() - 2;
    m_objVector[lastObjectIndex]->AddChild(m_objVector.back().get());

    m_objVector[0]->AddChild(m_objVector.back().get());

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
        WriteJson("CameraSaveTest.json");
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

    m_ui->BlurredViewportStart(m_DevicePtr.Get(), m_DevConPtr.Get());
    m_ui->GaussianBlur(m_DevConPtr.Get());
    m_ui->BlurredViewportUpdate(m_DevConPtr.Get());

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

bool ApplicationLayer::ReadJson(std::string path)
{
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
        XMFLOAT3 translation, rotation, scale;

        json& objectDesc = objects.at(i);
        std::string modelPath = objectDesc["ModelPath"];
        std::string texturePath = objectDesc["BaseColourPath"];

        std::string objName = objectDesc["Name"];

        // Maybe it's worth creating a read JSON Vector function to clean up sections like this.
        translation.x = objectDesc["PosX"]; translation.y = objectDesc["PosY"]; translation.z = objectDesc["PosZ"];
        rotation.x = objectDesc["RotX"]; rotation.y = objectDesc["RotY"]; rotation.z = objectDesc["RotZ"];
        scale.x = objectDesc["ScaleX"]; scale.y = objectDesc["ScaleY"]; scale.z = objectDesc["ScaleZ"];

        m_objVector.emplace_back(std::make_shared<GameObject>(m_DevicePtr.Get(), m_WindowHandle));
        m_objVector[i]->LoadModel(m_DevicePtr.Get(), m_DevConPtr.Get(), (char*)modelPath.c_str(), (char*)texturePath.c_str());
        m_objVector[i]->m_name = objName;

        m_objVector[i]->m_transform->m_position = translation;
        m_objVector[i]->m_transform->m_rotation = rotation;
        m_objVector[i]->m_transform->m_scale = scale;
    }

    json& cams = jFile["Cameras"];
    size = cams.size();
    m_camVector.reserve(size);
    for (int i = 0; i < size; i++)
    {
        XMFLOAT3 eye, at, up;

        json& camDesc = cams.at(i);

        std::string camType = camDesc["Camera Type"];

        eye.x = camDesc["EyeX"]; eye.y = camDesc["EyeY"]; eye.z = camDesc["EyeZ"];
        at.x = camDesc["AtX"]; at.y = camDesc["AtY"]; at.z = camDesc["AtZ"];
        up.x = camDesc["UpX"]; up.y = camDesc["UpY"]; up.z = camDesc["UpZ"];

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
        else if (camType == "Dolly")
        {
            std::shared_ptr<DollyCamera> dollyPtr = std::make_shared<DollyCamera>(eye, at, up,
                1280, 768, 0.01f, 100.0f);
            
            float speed = camDesc["Speed"];
            bool automatic = camDesc["MoveAutomatically"];
            bool retrace = camDesc["RetraceTrack"];

            dollyPtr->Initialise(speed, automatic, retrace);

            int pathSize = camDesc["PathSize"];

            json& path = camDesc["Path"];
            for (int i = 0; i < pathSize; i++)
            {
                json& point = path.at(i);

                float x = point["X"];
                float y = point["Y"];
                float z = point["Z"];

                dollyPtr->AddPoint(XMFLOAT3(x, y, z), i);
            }
            m_camVector.push_back(dollyPtr);
        }
    }
    return true;
}

bool ApplicationLayer::WriteJson(std::string path)
{
    std::ofstream fileWrite(path);

    json gameObjectsJson = json::array();

    for (int i = 0; i < m_objVector.size(); i++)
    {
            std::string modelPath = m_objVector[i]->m_modelPath;
            std::string baseColourPath = m_objVector[i]->m_texturePath;
            std::string name = m_objVector[i]->m_name;

            float posX = m_objVector[i]->m_transform->m_position.x;
            float posY = m_objVector[i]->m_transform->m_position.y;
            float posZ = m_objVector[i]->m_transform->m_position.z;
            float rotX = m_objVector[i]->m_transform->m_rotation.x;
            float rotY = m_objVector[i]->m_transform->m_rotation.y;
            float rotZ = m_objVector[i]->m_transform->m_rotation.z;
            float scaleX = m_objVector[i]->m_transform->m_scale.x;
            float scaleY = m_objVector[i]->m_transform->m_scale.y;
            float scaleZ = m_objVector[i]->m_transform->m_scale.z;

        gameObjectsJson.push_back(json::object(
            {
                    {"ModelPath", modelPath},
                    {"BaseColourPath", baseColourPath},

                    {"Name", name},

                    {"PosX", posX},
                    {"PosY", posY},
                    {"PosZ", posZ},
                    {"RotX", rotX},
                    {"RotY", rotY},
                    {"RotZ", rotZ},
                    {"ScaleX", scaleX},
                    {"ScaleY", scaleY},
                    {"ScaleZ", scaleZ},
            }));
    }

    json camerasJson = json::array();
    for (std::shared_ptr<BaseCamera> cam : m_camVector)
    {
        std::string camType = cam->GetType();

        float eyeX = cam->GetEye().x;
        float eyeY = cam->GetEye().y;
        float eyeZ = cam->GetEye().z;
        float atX = cam->GetAt().x;
        float atY = cam->GetAt().y;
        float atZ = cam->GetAt().z;
        float upX = cam->GetUp().x;
        float upY = cam->GetUp().y;
        float upZ = cam->GetUp().z;

        if (camType == "Base")
        {
            camerasJson.push_back(json::object(
                {
                    {"Camera Type", camType},

                    {"EyeX", eyeX},
                    {"EyeY", eyeY},
                    {"EyeZ", eyeZ},
                    {"AtX", atX},
                    {"AtY", atY},
                    {"AtZ", atZ},
                    {"UpX", upX},
                    {"UpY", upY},
                    {"UpZ", upZ}
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

                    {"EyeX", eyeX},
                    {"EyeY", eyeY},
                    {"EyeZ", eyeZ},
                    {"AtX", atX},
                    {"AtY", atY},
                    {"AtZ", atZ},
                    {"UpX", upX},
                    {"UpY", upY},
                    {"UpZ", upZ},

                    {"Yaw", yaw},
                    {"Pitch", pitch},

                    {"Speed", speed},
                    {"Sensitivity", sensitivty}
                }));
        }

        else if (camType == "Dolly")
        {
            DollyCamera* dollyCam = (DollyCamera*)cam.get();

            json dollyPath = json::array();
            int pathSize = dollyCam->GetPathSize();

            // TODO: Super duper cache unfriendly fix if I have time.
            for (int i = 0; i < pathSize; i++)
            {
                XMFLOAT3 point = dollyCam->GetPoint(i);

                dollyPath.push_back(json::object(
                    {
                        { "X", point.x },
                        { "Y", point.y },
                        { "Z", point.z }
                    }));
            }

            json cameraObj = json::object({
                {"Camera Type", camType},

                {"EyeX", eyeX},
                {"EyeY", eyeY},
                {"EyeZ", eyeZ},
                {"AtX", atX},
                {"AtY", atY},
                {"AtZ", atZ},
                {"UpX", upX},
                {"UpY", upY},
                {"UpZ", upZ},

                {"Speed", dollyCam->m_speed },
                {"RetraceTrack", dollyCam->m_retraceTrack },
                {"MoveAutomatically", dollyCam->m_moveAutomatically },

                { "Path", dollyPath },
                { "PathSize", pathSize }
                });

            camerasJson.push_back(cameraObj);
        }

        // Add anymore types down here.
    }

    fileWrite << json::object({
        {"GameObjects", gameObjectsJson},
        {"Cameras", camerasJson},
        {"Active Cam", m_activeCamera},
        {"SceneName", "Osaka"},
        }).dump(2) << std::endl;

    return true;
}