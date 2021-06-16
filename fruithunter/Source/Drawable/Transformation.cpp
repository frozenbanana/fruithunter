#include "Transformation.h"
#include "Renderer.h"
#include "ErrorLogger.h"

Microsoft::WRL::ComPtr<ID3D11Buffer> Transformation::m_modelMatrixBuffer;

Transformation::Transformation(float3 position, float3 scale, float3 rotation) {
	createBuffer();

	m_position = position;
	m_scale = scale;
	m_rotation = rotation;
	updateMatrix();
}

Transformation::~Transformation() {}

void Transformation::updateBuffer() {
	if (m_propertiesChanged)
		updateMatrix();
	// update buffer
	ModelMatrix mat = m_matrixBufferData;
	mat.matWorld = mat.matWorld.Transpose();
	mat.matInvTraWorld = mat.matInvTraWorld.Transpose();
	Renderer::getDeviceContext()->UpdateSubresource(m_modelMatrixBuffer.Get(), 0, 0, &mat, 0, 0);
}

void Transformation::updateMatrix() { 
	m_propertiesChanged = false;
	//update matrix
	float4x4 matWorld = getScalingMatrix() * getRotationMatrix() * getTranslateMatrix();
	m_matrixBufferData.matWorld = matWorld;
	m_matrixBufferData.matInvTraWorld = matWorld.Invert().Transpose();
}

void Transformation::createBuffer() {
	// matrix buffer
	if (m_modelMatrixBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(ModelMatrix);

		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_modelMatrixBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError("(Transform) Failed creating matrix buffer!\n", res);
	}
}

float3 Transformation::getPosition() const { return m_position; }

float3 Transformation::getScale() const { return m_scale; }

float3 Transformation::getRotation() const { return m_rotation; }

float4x4 Transformation::getMatrix() { 
	if (m_propertiesChanged)
		updateMatrix();
	return m_matrixBufferData.matWorld; 
}

float4x4 Transformation::getInversedTransposedMatrix() {
	if (m_propertiesChanged)
		updateMatrix();
	return m_matrixBufferData.matInvTraWorld;
}

float4x4 Transformation::getTranslateMatrix() const {
	return float4x4::CreateTranslation(m_position);
}

float4x4 Transformation::getScalingMatrix() const { return float4x4::CreateScale(m_scale); }

float4x4 Transformation::getRotationMatrix() const {
	return float4x4::CreateRotationZ(m_rotation.z) * float4x4::CreateRotationX(m_rotation.x) *
		   float4x4::CreateRotationY(m_rotation.y);
	;
}

void Transformation::setPosition(float3 position) { 
	m_position = position;
	m_propertiesChanged = true;
}

void Transformation::setScale(float3 scale) { 
	m_scale = scale;
	m_propertiesChanged = true;
}

void Transformation::setScale(float scale) { setScale(float3(1.) * scale); }

void Transformation::setRotation(float3 rotation) {
	m_rotation = rotation;
	m_propertiesChanged = true;

}

void Transformation::rescale(float scale) { setScale(getScale() * scale); }

void Transformation::rescale(float3 scale) { setScale(getScale() * scale); }

void Transformation::move(float3 movement) { setPosition(getPosition() + movement); }

void Transformation::rotate(float3 rotation) { setRotation(getRotation() + rotation); }

void Transformation::rotateX(float value) { setRotation(getRotation() + float3(value, 0, 0)); }

void Transformation::rotateY(float value) { setRotation(getRotation() + float3(0, value, 0)); }

void Transformation::rotateZ(float value) { setRotation(getRotation() + float3(0, 0, value)); }

void Transformation::lookAt(float3 target) {
	float3 dir = target - m_position;
	dir.Normalize();

	float2 mapY(dir.z, dir.x);
	float2 mapX(float2(dir.x, dir.z).Length(), dir.y);
	mapY.Normalize();
	mapX.Normalize();
	float rotY = (mapY.y >= 0) ? (acos(mapY.x)) : (-acos(mapY.x));
	float rotX = -((mapX.y >= 0) ? (acos(mapX.x)) : (-acos(mapX.x)));
	setRotation(float3(rotX, rotY, 0));
}

void Transformation::lookTo(float3 direction) { lookAt(direction+m_position); }

void Transformation::VSBindMatrix(size_t indexRegister) {
	updateBuffer();
	Renderer::getDeviceContext()->VSSetConstantBuffers(
		indexRegister, 1, m_modelMatrixBuffer.GetAddressOf());
}

void Transformation::GSBindMatrix(size_t indexRegister) {
	updateBuffer();
	Renderer::getDeviceContext()->GSSetConstantBuffers(
		indexRegister, 1, m_modelMatrixBuffer.GetAddressOf());
}

void Transformation::PSBindMatrix(size_t indexRegister) {
	updateBuffer();
	Renderer::getDeviceContext()->PSSetConstantBuffers(
		indexRegister, 1, m_modelMatrixBuffer.GetAddressOf());
}

void Transformation::stream_write(fstream& file) {
	file.write((char*)&m_position, sizeof(float3));
	file.write((char*)&m_rotation, sizeof(float3));
	file.write((char*)&m_scale, sizeof(float3));
}

void Transformation::stream_read(fstream& file) {
	file.read((char*)&m_position, sizeof(float3));
	file.read((char*)&m_rotation, sizeof(float3));
	file.read((char*)&m_scale, sizeof(float3));
	m_propertiesChanged = true;
}

void Transformation::imgui_properties() {
	if (ImGui::InputFloat3("Position", (float*)&m_position))
		m_propertiesChanged = true;
	if (ImGui::InputFloat3("Rotation", (float*)&m_rotation))
		m_propertiesChanged = true;
	if (ImGui::InputFloat3("Scale", (float*)&m_scale))
		m_propertiesChanged = true;
}
