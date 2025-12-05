#include "DollyCamera.h"
#include "Input.h"
#include <algorithm>

using namespace DirectX;

DollyCamera::DollyCamera(XMFLOAT3 position, XMFLOAT3 at, XMFLOAT3 up,
    float windowWidth, float windowHeight, float near, float far) : 

    BaseCamera(position, at, up, windowWidth, windowHeight , near, far)
{
    m_type = "Dolly";
}

DollyCamera::~DollyCamera()
{
}

void DollyCamera::Initialise(float speed, bool moveAutomatically, bool retraceTrack)
{
    m_speed = speed;
    m_moveAutomatically = moveAutomatically;
    m_retraceTrack = retraceTrack;
}

void DollyCamera::Update(float deltaTime)
{
    if(m_moveAutomatically)
    {
        if (m_percent >= 1.0f)
        {
            if (m_retraceTrack)
                m_isRetracing = true;
            else
                m_percent = 0;
        }
        else if (m_percent <= 0.0f)
        {
            if (m_retraceTrack)
                m_isRetracing = false;
            else
                m_percent = 0;
        }
    
        if (!m_isRetracing)
            m_percent += m_speed * deltaTime;
        else
            m_percent -= m_speed * deltaTime;
    }
    else
    {
        if (Input::GetKeyHeld('A'))
        {
            m_percent -= m_speed * deltaTime;
        }
        else if (Input::GetKeyHeld('D'))
        {
            m_percent += m_speed * deltaTime;
        }

        m_percent = std::clamp(m_percent, 0.0f, 1.0f);
    }


    m_eye = Bezier(m_path, m_percent);

    XMVECTOR camPos = XMLoadFloat3(&m_eye);
    XMVECTOR atVector = XMLoadFloat3(&m_at);
    XMVECTOR upVector = XMLoadFloat3(&m_up);

    XMStoreFloat4x4(&m_view, XMMatrixLookToLH(camPos, atVector, upVector));
}

void DollyCamera::AddPoint(DirectX::XMFLOAT3 point, int position)
{
    if (position >= m_path.size())
    {
        m_path.push_back(point);
    }
    else
    {
        m_path.insert(m_path.begin() + position, point);
    }
}

XMFLOAT3 DollyCamera::GetPoint(int index)
{
    return m_path[index];
}

int DollyCamera::GetPathSize()
{
    return m_path.size();
}

XMFLOAT3 DollyCamera::Bezier(std::vector<DirectX::XMFLOAT3>& path, float percent)
{
    XMFLOAT3 result = XMFLOAT3(0.0f, 0.0f, 0.0f);

    int pathSize = path.size() - 1;

    for (int i = 0; i < pathSize + 1; i++)
    {
        float coefficent = Binomial(pathSize, i) * powf(1 - percent, pathSize - i) * powf(percent, i);

        result.x += coefficent * path[i].x;
        result.y += coefficent * path[i].y;
        result.z += coefficent * path[i].z;
    }

    return result;
}

float DollyCamera::Binomial(float n, float k)
{
    float res = 1.0f;

    for (int i = 1; i < k + 1; i++)
    {
        res *= n - (k - i);
        res /= i;
    }

    return res;
}