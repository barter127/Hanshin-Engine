#include "ApplicationLayer.h"

#include "Texture.h"
#include <algorithm>
#include "ModelComponent.h"
#include "ImGuiWrapper.h"
#include "DX11Framework.h"
#include "TransformComponent.h"

#include "DirectionalLight.h"
#include "PointLight.h"

#include "SkyBox.h"
#include "Input.h"
#include "Saving.h"

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

    Saving::LoadSceneFromJSON("MainScene.json", m_objVector, m_camVector, device, devCon, windowHandle);

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

	m_objVector[0].get()->AddChild(m_objVector[1].get());
	m_objVector[1].get()->AddChild(m_objVector[2].get());
	m_objVector[0].get()->AddChild(m_objVector[3].get());

	// Idk how I feel about this way of setting ids. Maybe we could save it 
	for (int i = 0; i < m_objVector.size(); i++)
	{
		m_objVector[i].get()->m_id = i;
	}
	GameObject::m_nextID = m_objVector.size();

    return true;
}

void ApplicationLayer::Update(float deltaTime)
{
    m_camVector[m_activeCamera]->Update(deltaTime);

    m_ui->StartUpdate(deltaTime);

    switch (m_ui->NewObjectPanel())
    {
    case m_ui->Primitives::Cube:
        CreateNewObject((char*)"Primitives/Prim_Cube.obj");
        break;

    case m_ui->Primitives::Plane:
        CreateNewObject((char*)"Primitives/Prim_Plane.obj");
        break;

    case m_ui->Primitives::Sphere:
        CreateNewObject((char*)"Primitives/Prim_Sphere.obj");
        break;

    case m_ui->Primitives::Cylinder:
        CreateNewObject((char*)"Primitives/Prim_Cylinder.obj");
        break;

    case m_ui->Primitives::Cone:
        CreateNewObject((char*)"Primitives/Prim_Cone.obj");
        break;

    case m_ui->Primitives::Torus:
        CreateNewObject((char*)"Primitives/Prim_Torus.obj");
        break;

    case m_ui->Primitives::Dragon:
        CreateNewObject((char*)"Primitives/dragon.obj");
        break;
    }

    m_ui->TransformPanel(*m_objVector[0]);


    //m_ui->LightPanel(m_pointLight->GetAmbientColour(),
    //    m_pointLight->GetDiffuseColour(),
    //    m_pointLight->GetSpecularColour(), m_pointLight->GetSpecularPower(),
    //    m_pointLight->GetLightPos());

	m_ui->SceneGraph(m_objVector);

    m_ui->ContentBrowser();

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

    if (m_ui->SaveChanges())
    {
        Saving::SaveSceneFromJSON("MainScene.json", m_objVector, m_camVector);
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
    m_ui->HandleTransformGizmo(*m_objVector[m_activeCamera], m_camVector[m_activeCamera]->GetView(), m_camVector[m_activeCamera]->GetProj());

    //m_ui->BlurredViewportStart(m_DevicePtr.Get(), m_DevConPtr.Get());
    //m_ui->GaussianBlur(m_DevConPtr.Get());
    //m_ui->BlurredViewportUpdate(m_DevConPtr.Get());

    dx->SetBackBufferRenderTarget();
    dx->ResetViewport();

    m_ui->Render();
}

void ApplicationLayer::CreateNewObject(char* modelPath)
{
    m_objVector.emplace_back(std::make_shared<GameObject>(m_DevicePtr.Get(), m_WindowHandle));
    bool result = m_objVector.back()->LoadModel(m_DevicePtr.Get(), m_DevConPtr.Get(), modelPath);
}