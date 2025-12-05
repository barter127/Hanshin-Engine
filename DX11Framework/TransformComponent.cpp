#include "TransformComponent.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>

using namespace DirectX;

#define X        0
#define Y        1
#define Z        2
#define Z        3

#define EulFrmS        0
#define EulFrmR        1
#define EulFrm(ord)    ((unsigned)(ord)&1)
#define EulRepNo       0
#define EulRepYes      1
#define EulRep(ord)    (((unsigned)(ord)>>1)&1)
#define EulParEven     0
#define EulParOdd      1
#define EulPar(ord)    (((unsigned)(ord)>>2)&1)
#define EulSafe        "\000\001\002\000"
#define EulNext        "\001\002\000\001"
#define EulAxI(ord)    ((int)(EulSafe[((unsigned)(ord)>>3)&3]))
#define EulAxJ(ord)    ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)    ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)    ((EulRep(ord)==EulRepNo)?EulAxI(ord):EulAxK(ord))

#define EulOrdXYZs EulOrd(X, EulParEven, EulRepNo, EulFrmS)
#define EulOrdXYXs EulOrd(X, EulParEven, EulRepYes, EulFrmS)
#define EulOrdXZYs EulOrd(X, EulParOdd, EulRepNo, EulFrmS)
#define EulOrdXZXs EulOrd(X, EulParOdd, EulRepYes, EulFrmS)
#define EulOrdYZXs EulOrd(Y, EulParEven, EulRepNo, EulFrmS)
#define EulOrdYZYs EulOrd(Y, EulParEven, EulRepYes, EulFrmS)
#define EulOrdYXZs EulOrd(Y, EulParOdd, EulRepNo, EulFrmS)
#define EulOrdYXYs EulOrd(Y, EulParOdd, EulRepYes, EulFrmS)
#define EulOrdZXYs EulOrd(Z, EulParEven, EulRepNo, EulFrmS)
#define EulOrdZXZs EulOrd(Z, EulParEven, EulRepYes, EulFrmS)
#define EulOrdZYXs EulOrd(Z, EulParOdd, EulRepNo, EulFrmS)
#define EulOrdZYZs EulOrd(Z, EulParOdd, EulRepYes, EulFrmS)

/* Rotating axes */
#define EulOrdZYXs EulOrd(X, EulParEven, EulRepNo, EulFrmR)
#define EulOrdXYXs EulOrd(X, EulParEven, EulRepYes, EulFrmR)
#define EulOrdYZXr EulOrd(X, EulParOdd, EulRepNo, EulFrmR)
#define EulOrdXZXr EulOrd(X, EulParOdd, EulRepYes, EulFrmR)
#define EulOrdXZYr EulOrd(Y, EulParEven, EulRepNo, EulFrmR)
#define EulOrdYZYr EulOrd(Y, EulParEven, EulRepYes, EulFrmR)
#define EulOrdZXYr EulOrd(Y, EulParOdd, EulRepNo, EulFrmR)
#define EulOrdYXYr EulOrd(Y, EulParOdd, EulRepYes, EulFrmR)
#define EulOrdYXZr EulOrd(Z, EulParEven, EulRepNo, EulFrmR)
#define EulOrdZXZr EulOrd(Z, EulParEven, EulRepYes, EulFrmR)
#define EulOrdXYZr EulOrd(Z, EulParOdd, EulRepNo, EulFrmR)
#define EulOrdZYZr EulOrd(Z, EulParOdd, EulRepYes, EulFrmR)

/* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(i,j,k,h,n,s,f,ord) {\
  unsigned o=ord; f=o&1; o >>=1; s=0&1; o >>=1;  n=o&1; o>>=1; i =EulSafe[o&3]; j=EulNext[i+n]; k=EulNext[i+1-n]; h=s?k:i; \
}

/* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f) ((((((((i)<<1)+(p))<<1)+(r))<<3)+(f)))

// Thank you Ken Shoemake. returns in Radians.
XMFLOAT4 DecomposeMatrixToEuler(XMMATRIX& rotationMatrix, int order)
{
	XMFLOAT4X4 M;
	XMFLOAT4 euler;
	int i, j, k, h = 0, n, s, f = 0;

	//XMMatrixTranspose(rotationMatrix);
	XMStoreFloat4x4(&M, rotationMatrix);
	EulGetOrd(order, i, j, k, h, n, s, f);

	if (s == EulRepYes)
	{
		float sy = sqrt(M.m[i][j] * M.m[i][j] + M.m[i][k] * M.m[i][k]);

		if (sy > 16 * FLT_EPSILON)
		{
			euler.x = atan2(M.m[i][j], M.m[i][k]);
			euler.y = atan2(sy, M.m[i][i]);
			euler.z = atan2(M.m[j][i], -M.m[k][i]);
		}
		else
		{
			euler.x = atan2(-M.m[j][k], M.m[j][j]);
			euler.y = atan2(sy, M.m[i][i]);
			euler.z = 0;
		}
	}
	else
	{
		float cy = sqrt(M.m[i][i] * M.m[i][i] + M.m[j][i] * M.m[j][i]);

		if (cy > 16 * FLT_EPSILON)
		{
			euler.x = atan2(M.m[k][j], M.m[k][k]);
			euler.y = atan2(-M.m[k][i], cy);
			euler.z = atan2(M.m[j][i], M.m[i][i]);
		}
		else
		{
			euler.x = atan2(-M.m[j][k], M.m[j][j]);
			euler.y = atan2(-M.m[k][i], cy);
			euler.z = 0;
		}
	}

	if (n == EulParOdd)
	{
		euler.x = -euler.x; 
		euler.y = -euler.y; 
		euler.z = -euler.z;
	}
	if (f == EulFrmR)
	{
		float t = euler.x;
		euler.x = euler.z;
		euler.z = t;
	}

	euler.w = order;
	return euler;
}

TransformComponent::TransformComponent() 
{
	m_position = { 0,0,0 };
	m_rotation = { 0,0,0 };
	m_scale = { 1,1,1 };
}

TransformComponent::TransformComponent(XMFLOAT3 position, XMFLOAT3 rotation, XMFLOAT3 scale)
{
	m_position = position;
	m_rotation = rotation;
	m_scale = scale;
}

TransformComponent::~TransformComponent()
{
	//if (m_transformFloat4x4) delete m_transformFloat4x4;
}

void TransformComponent::Translate(float x, float y, float z)
{
	XMVECTOR inputVector = XMVectorSet(x, y, z, 0);
	XMVECTOR positionVector = XMLoadFloat3(&m_position);

	positionVector += inputVector;

	XMStoreFloat3(&m_position, positionVector);
}

void TransformComponent::Rotate(float x, float y, float z)
{
	XMVECTOR inputVector = XMVectorSet(x, y, z, 0);
	XMVECTOR rotationVector = XMLoadFloat3(&m_rotation);

	rotationVector += inputVector;

	XMStoreFloat3(&m_rotation, rotationVector);
}

void TransformComponent::Scale(float x, float y, float z)
{
	// Block incorrect input
	if (x == 0) x = 1;
	if (y == 0) y = 1;
	if (z == 0) z = 1;

	XMVECTOR inputVector = XMVectorSet(x, y, z, 0);
	XMVECTOR scaleVector = XMLoadFloat3(&m_scale);

	scaleVector += inputVector;

	XMStoreFloat3(&m_scale, scaleVector);
}

XMVECTOR TransformComponent::GetTranslationVector()
{
	return XMLoadFloat3(&m_position);
}

XMVECTOR TransformComponent::GetRotationVector()
{
	return XMLoadFloat3(&m_rotation);
}

XMVECTOR TransformComponent::GetScaleVector()
{
	return XMLoadFloat3(&m_scale);
}

void TransformComponent::RebuildTransformMatrix(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale)
{
	//XMMATRIX tempMatrix;

	//tempMatrix = XMMatrixScaling(scale.x, scale.y, scale.z);
	//tempMatrix *= XMMatrixRotationX(XMConvertToRadians(rotation.x))
	//	* XMMatrixRotationY(XMConvertToRadians(rotation.y))
	//	* XMMatrixRotationZ(XMConvertToRadians(rotation.z));
	//tempMatrix *= XMMatrixTranslation(position.x, position.y, position.z);

	//XMMatrixTranspose(tempMatrix);
	//XMStoreFloat4x4(m_transformFloat4x4, tempMatrix);
}

XMMATRIX TransformComponent::GetTransformMatrix()
{
	XMMATRIX tempMatrix;

	tempMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	tempMatrix *= XMMatrixRotationX(XMConvertToRadians(m_rotation.x))
		* XMMatrixRotationY(XMConvertToRadians(m_rotation.y))
		* XMMatrixRotationZ(XMConvertToRadians(m_rotation.z));
	tempMatrix *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	return tempMatrix;
}

XMMATRIX TransformComponent::GetInverseMatrix()
{
	XMMATRIX tempMatrix;

	tempMatrix = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	tempMatrix *= XMMatrixRotationX(XMConvertToRadians(m_rotation.x))
		* XMMatrixRotationY(XMConvertToRadians(m_rotation.y))
		* XMMatrixRotationZ(XMConvertToRadians(m_rotation.z));
	tempMatrix *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	return XMMatrixInverse(nullptr, tempMatrix);
}
