#pragma once
#include "BaseCamera.h"
#include <vector>
#include <directxmath.h>

class DollyCamera : public BaseCamera
{
public:
	DollyCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 at, DirectX::XMFLOAT3 up,
		float windowWidth, float windowHeight, float near, float far);
	~DollyCamera();

	void Initialise(float speed, bool moveAutomatically, bool retraceTrack);

	void Update(float deltaTime) override;

	void AddPoint(DirectX::XMFLOAT3, int position);
	DirectX::XMFLOAT3 GetPoint(int index);
	int GetPathSize();

public:
	// Speed camera moves along the track.
	float m_speed = 0.5f;

	// Does cam move automatically down track.
	bool m_moveAutomatically = true;

	// Does the bezier move back round the track or snap to the start.
	bool m_retraceTrack = true;

private:
	static float Binomial(float n, float k);
	static DirectX::XMFLOAT3 Bezier(std::vector<DirectX::XMFLOAT3>& path, float percent);

private:
	std::vector<DirectX::XMFLOAT3> m_path;

	float m_percent = 0.0f;
	bool m_isRetracing = false;
};