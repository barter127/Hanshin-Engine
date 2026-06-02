#include "Saving.h"

#include <fstream>

#include <DirectXMath.h>

#include "GameObject.h"
#include "TransformComponent.h"
#include "DebugCamera.h"

#include "json.hpp"
using json = nlohmann::json;

using namespace std;
using namespace DirectX;


bool Saving::LoadSceneFromJSON(string path, vector<shared_ptr<GameObject>>& sceneVector, std::vector<std::shared_ptr<BaseCamera>>& camVector, 
	ID3D11Device* device, ID3D11DeviceContext* devCon, HWND winHandle)
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

	json& objects = jFile["GameObjects"];

	int size = objects.size();
	sceneVector.reserve(size);
	for (int i = 0; i < size; i++)
	{
		//Vector3 translation, rotation, scale;

		json& objectDesc = objects.at(i);
		std::string modelPath = objectDesc["ModelPath"];
		std::string objName = objectDesc["Name"];

		sceneVector.emplace_back(std::make_shared<GameObject>(device, winHandle));
		sceneVector.back().get()->LoadModel(device, devCon, (char*)modelPath.c_str());
		sceneVector[i]->m_name = objName;

		TransformComponent* transform = sceneVector.back()->GetTransform();
		transform->m_position = ReadXMFloat3("Position", objectDesc);
		transform->m_rotation = (ReadXMFloat3("Rotation", objectDesc));
		transform->m_scale = ReadXMFloat3("Scale", objectDesc);
	}

	json& cams = jFile["Cameras"];
	size = cams.size();
	camVector.reserve(size);
	for (int i = 0; i < size; i++)
	{
		json& camDesc = cams.at(i);

		CameraTypes camType = camDesc["Camera Type"];

		XMFLOAT3 eye = ReadXMFloat3("Eye", camDesc);
		XMFLOAT3 at = ReadXMFloat3("At", camDesc);
		XMFLOAT3 up = ReadXMFloat3("Up", camDesc);

		switch (camType)
		{
			case CameraTypes::Base:
				camVector.push_back(std::make_shared<BaseCamera>(eye, at, up,
					1280, 768, 0.01f, 100.0f));

			case CameraTypes::Debug:
			{
				float yaw = camDesc["Yaw"];
				float pitch = camDesc["Pitch"];
				float sensitivity = camDesc["Sensitivity"];
				float speed = camDesc["Speed"];

				std::shared_ptr<DebugCamera> debugPtr = std::make_shared<DebugCamera>(eye, at, up,
					1280, 768, 0.01f, 100.0f);
				debugPtr->Initialise(yaw, pitch, sensitivity, speed);

				camVector.push_back(debugPtr);
			}
		}
	}
	return true;
}

bool Saving::SaveSceneFromJSON(string path, vector<shared_ptr<GameObject>>& sceneVector, std::vector<std::shared_ptr<BaseCamera>>& camVector)
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

	for (int i = 0; i < sceneVector.size(); i++)
	{
		std::string modelPath = sceneVector[i]->m_modelPath;
		std::string name = sceneVector[i]->m_name;

		TransformComponent* transform = sceneVector[i]->GetTransform();

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
	for (std::shared_ptr<BaseCamera> cam : camVector)
	{
		CameraTypes camType = cam->GetType();

		switch (camType)
		{
			case CameraTypes::Base:
			{
				camerasJson.push_back(json::object(
					{
						{"Camera Type", camType},

						SaveXMFloat3("Eye", cam->GetEye()),
						SaveXMFloat3("At", cam->GetAt()),
						SaveXMFloat3("Up", cam->GetUp()),
					}));
			}

			case CameraTypes::Debug:
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

			case CameraTypes::Dolly:
		}

		fileWrite << json::object({
			{"GameObjects", gameObjectsJson},
			{"Cameras", camerasJson},
			{"Active Cam", 0},
			{"SceneName", "Osaka"},
			}).dump(2) << std::endl;

		return true;
	}
}
