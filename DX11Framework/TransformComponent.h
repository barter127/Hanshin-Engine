#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <d3d11.h>
#include <directxmath.h>

class TransformComponent
{
public:
	// Construct transform matrix using identity.
	TransformComponent();

	// Construct transform matrix with predefined position, rotation, and scale.
	TransformComponent(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);
	~TransformComponent();

	/* \brief Translate the current transformation by the input parameters.
	* \param x The X translation in cartesian.
	* \param y The Y translation in cartesian.
	* \param z The Z translation in cartesian.
	*/
	void Translate(float x, float y, float z);

	/* \brief Rotate the current transformation by the input parameters.
	* \param x The X rotation in degrees as euler.
	* \param y The Y rotation in degrees as euler.
	* \param z The Z rotation in degrees as euler.
	*/
	void Rotate(float x, float y, float z);

	/* \brief Scale the current transformation by the input parameters.
	* \param x The X scale in cartesian.
	* \param y The Y scale in cartesian.
	* \param z The Z scale in cartesian.
	*/
	void Scale(float x, float y, float z);

	// \return The translation vector decomposed from the transform matrix in cartesian.
	DirectX::XMVECTOR GetTranslationVector();
	// \return The Rotation vector decomposed from the transform matrix as euler.
	DirectX::XMVECTOR GetRotationVector();
	// \return The Scale vector decomposed from the transform matrix in cartesian.
	DirectX::XMVECTOR GetScaleVector();

	void RebuildTransformMatrix(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 rotation, DirectX::XMFLOAT3 scale);
	// \return The current transformation matrix.
	DirectX::XMMATRIX GetTransformMatrix();
	DirectX::XMMATRIX GetInverseMatrix();

public:
	DirectX::XMFLOAT3 m_position;
	DirectX::XMFLOAT3 m_rotation;
	DirectX::XMFLOAT3 m_scale;
};

#endif // !TRANSFORM_COMPONENT_H