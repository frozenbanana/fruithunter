#pragma once
#include "GlobalNameSpaces.h"
class Transformation {
private:
	float3 m_position;
	float3 m_scale;
	float3 m_rotation;
	bool m_propertiesChanged = true;

	struct ModelMatrix {
		float4x4 matWorld, matInvTraWorld;
	} m_matrixBufferData; // matrix for translation, rotation and scale
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_modelMatrixBuffer;

	//-- Private Functions --

	void updateMatrix();
	void updateBuffer();
	void createBuffer();

public:
	float3 getPosition() const;
	float3 getScale() const;
	float3 getRotation() const;
	float4x4 getMatrix();
	float4x4 getInversedTransposedMatrix();
	float4x4 getTranslateMatrix() const;
	float4x4 getScalingMatrix() const;
	float4x4 getRotationMatrix() const;

	virtual void setPosition(float3 position);
	virtual void setScale(float3 scale);
	virtual void setRotation(float3 rotation);
	void setScale(float scale);

	void rescale(float scale);
	void rescale(float3 scale);
	void move(float3 movement);
	/* Multiplication order -> ZXY */
	void rotate(float3 rotation);
	void rotateX(float value);
	void rotateY(float value);
	void rotateZ(float value);
	/* Change rotation to be facing target position. */
	void lookAt(float3 target);
	/* Change rotation to be facing specified direction. */
	void lookTo(float3 direction);

	/* Bind world&invWorld matrix to Vertex Shader. */
	void VSBindMatrix(size_t indexRegister);
	/* Bind world&invWorld matrix to Geometry Shader. */
	void GSBindMatrix(size_t indexRegister);
	/* Bind world&invWorld matrix to Pixel Shader. */
	void PSBindMatrix(size_t indexRegister);

	void stream_write(ofstream& file);
	void stream_read(ifstream& file);

	void imgui_properties();

	Transformation(float3 position = float3(0.), float3 scale = float3(1.), float3 rotation = float3(0.));
	virtual ~Transformation();

};
