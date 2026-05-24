#pragma once

#include <vector>
#include <memory>
#include <string>

#include <d3d11.h>

class GameObject;
class BaseCamera;

namespace Saving
{
	bool LoadSceneFromJSON(std::string path, std::vector<std::shared_ptr<GameObject>>& sceneVector, std::vector<std::shared_ptr<BaseCamera>>& cameraVector, 
		ID3D11Device* device, ID3D11DeviceContext* devCon, HWND winHandle);

	bool SaveSceneFromJSON(std::string path, std::vector<std::shared_ptr<GameObject>>& sceneVector, std::vector<std::shared_ptr<BaseCamera>>& cameraVector);
};

