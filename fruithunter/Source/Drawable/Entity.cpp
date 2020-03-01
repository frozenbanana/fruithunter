#include "Entity.h"
#include "Errorlogger.h"
#include "Input.h"

void Entity::updateMatrix() {
	if (m_transformPropertiesChanged) {
		m_transformPropertiesChanged = false;
		float4x4 matTranslation = float4x4::CreateTranslation(m_position);
		float4x4 matScale = float4x4::CreateScale(m_scale);
		float4x4 matWorld = matScale * m_matRotation * matTranslation;
		m_matrixBufferData.matWorld = matWorld;
		m_matrixBufferData.matInvTraWorld = matWorld.Invert().Transpose();

		m_collisionData.setCollisionPosition(m_position);
		m_collisionData.setCollisionScale(m_scale);
		m_collisionData.rotateObbAxis(m_matRotation);
		// reset matricies
		// m_matRotation = XMMatrixIdentity();
	}
}

void Entity::bindModelMatrixBuffer() {
	if (m_transformPropertiesChanged) {
		// update matrix if needed
		updateMatrix();
	}
	// fill data to pipeline, transpose matrices
	Entity::MatrixBuffer transposedData = m_matrixBufferData;
	transposedData.matWorld = transposedData.matWorld.Transpose();
	transposedData.matInvTraWorld = transposedData.matInvTraWorld.Transpose();
	Renderer::getDeviceContext()->UpdateSubresource(
		m_modelMatrixBuffer.Get(), 0, 0, &transposedData, 0, 0);
	// bind buffer to pipeline
	Renderer::getDeviceContext()->VSSetConstantBuffers(
		MODEL_MATRIX_BUFFER_SLOT, 1, m_modelMatrixBuffer.GetAddressOf());
}

void Entity::createBuffers() {
	// matrix buffer
	if (m_modelMatrixBuffer.Get() == nullptr) {
		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(MatrixBuffer);

		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&desc, nullptr, m_modelMatrixBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(res, "Entity failed creating matrix buffer!");
	}
}

bool Entity::isMeshInitialized() const { return (m_mesh.get() != nullptr); }

bool Entity::atOrUnder(float terrainHeight) const {
	return m_position.y <= (terrainHeight + getHalfSizes().y / 2.f);
}

string Entity::getModelName() const {
	if (m_mesh != nullptr) {
		return m_mesh->getName();
	}
	return "";
}

float4x4 Entity::getModelMatrix() {
	updateMatrix();
	return m_matrixBufferData.matWorld;
}

float4x4 Entity::getRotationMatrix() const { return m_matRotation; }

float3 Entity::getPosition() const { return m_position; }

float3 Entity::getScale() const { return m_scale; }

float3 Entity::getLocalBoundingBoxPosition() const { 
	if (m_mesh.get() != nullptr)
		return m_mesh->getBoundingBoxPos();
	else
		return float3(0, 0, 0);
}

float3 Entity::getLocalBoundingBoxSize() const {
	if (m_mesh.get() != nullptr)
		return m_mesh->getBoundingBoxSize();
	else
		return float3(0, 0, 0);
}

void Entity::setPosition(float3 position) {
	m_position = position;
	m_transformPropertiesChanged = true;
}

void Entity::move(float3 movement) {
	m_position += movement;
	m_transformPropertiesChanged = true;
}

void Entity::setRotationMatrix(float4x4 matrix) {
	m_matRotation = matrix;
	m_transformPropertiesChanged = true;
}

void Entity::setRotationByAxis(float3 axis, float angle) {
	axis.Normalize();
	Quaternion q = Quaternion(axis, angle);
	q.Normalize();
	// ref:
	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
	float4x4 convertedMatrix = float4x4(float4(q.w, q.z, -q.y, q.x), float4(-q.z, q.w, q.x, q.y),
								   float4(q.y, -q.x, q.w, q.z), float4(-q.x, -q.y, -q.z, q.w)) *
							   float4x4(float4(q.w, q.z, -q.y, -q.x), float4(-q.z, q.w, q.x, -q.y),
								   float4(q.y, -q.x, q.w, -q.z), float4(q.x, q.y, q.z, q.w));

	m_matRotation = convertedMatrix;
	m_transformPropertiesChanged = true;
}

void Entity::setRotation(float3 rotation) {

	// m_matRotation = float4x4::CreateRotationX(rotation.x) * float4x4::CreateRotationY(rotation.y)
	// * 				float4x4::CreateRotationZ(rotation.z); m_transformPropertiesChanged = true;

	m_matRotation = XMMatrixIdentity();
	rotate(rotation);
}

void Entity::rotate(float3 rotation) {
	rotateZ(rotation.z);
	rotateX(rotation.x);
	rotateY(rotation.y);
}

void Entity::rotateByAxis(float3 axis, float angle) {
	axis.Normalize();
	Quaternion q = Quaternion(axis, angle);
	q.Normalize();
	// ref:
	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToMatrix/index.htm
	float4x4 convertedMatrix = float4x4(float4(q.w, q.z, -q.y, q.x), float4(-q.z, q.w, q.x, q.y),
								   float4(q.y, -q.x, q.w, q.z), float4(-q.x, -q.y, -q.z, q.w)) *

							   float4x4(float4(q.w, q.z, -q.y, -q.x), float4(-q.z, q.w, q.x, -q.y),
								   float4(q.y, -q.x, q.w, -q.z), float4(q.x, q.y, q.z, q.w));

	m_matRotation *= convertedMatrix;
	m_transformPropertiesChanged = true;
}

void Entity::rotateX(float val) {
	m_matRotation *= float4x4::CreateRotationX(val);
	m_transformPropertiesChanged = true;
}

void Entity::rotateY(float val) {
	m_matRotation *= float4x4::CreateRotationY(val);
	m_transformPropertiesChanged = true;
}

void Entity::rotateZ(float val) {
	m_matRotation *= float4x4::CreateRotationZ(val);
	m_transformPropertiesChanged = true;
}

void Entity::setScale(float3 scale) {
	m_scale = float3(abs(scale.x), abs(scale.y), abs(scale.z));
	scaleBoundingBoxHalfSizes(m_scale);
	m_transformPropertiesChanged = true;
}

void Entity::setScale(float scale) {
	m_scale = float3(abs(scale), abs(scale), abs(scale));
	scaleBoundingBoxHalfSizes(m_scale);
	m_transformPropertiesChanged = true;
}

void Entity::scaleBoundingBoxHalfSizes(float3 scale) { m_collisionData.setCollisionScale(scale); }

void Entity::draw() {
	if (isMeshInitialized()) {
		bindModelMatrixBuffer();

		if (Input::getInstance()->keyDown(Keyboard::B))
			m_mesh->draw_BoundingBox();

		m_mesh.get()->draw();
	}
}

void Entity::drawShadow() {
	if (isMeshInitialized()) {
		bindModelMatrixBuffer();

		m_mesh.get()->drawShadow();
	}
}

void Entity::draw_onlyMesh(float3 color) {
	if (isMeshInitialized()) {
		bindModelMatrixBuffer();
		m_mesh.get()->draw_noMaterial(color);
	}
}

void Entity::draw_boundingBox() {
	if (isMeshInitialized()) {
		bindModelMatrixBuffer();
		m_mesh.get()->draw_BoundingBox();
	}
}

void Entity::draw_animate() {
	bindModelMatrixBuffer();
	setMaterial(m_currentMaterial);
	m_meshAnim.draw();
}

void Entity::draw_animate_shadow() {
	bindModelMatrixBuffer();
	m_meshAnim.drawShadow();
}

void Entity::updateAnimated(float dt) { m_meshAnim.update(dt); }

void Entity::updateAnimatedSpecific(float frameTime) { m_meshAnim.updateSpecific(frameTime); }

void Entity::setFrameTargets(int first, int second) { m_meshAnim.setFrameTargets(first, second); }

bool Entity::load(string filename) {
	shared_ptr<Mesh> m = MeshRepository::get(filename);
	if (m.get() != nullptr) {
		m_mesh = m;
		return true;
	}
	else {
		return false;
	}
}

bool Entity::loadAnimated(string filename, int nrOfFrames) {
	return m_meshAnim.load(filename, nrOfFrames);
}

void Entity::setMaterial(int index) {
	// Asumes normal mesh won't change materials
	m_meshAnim.setMaterials(index);
}

void Entity::loadMaterials(std::vector<string> fileNames, int nrOfMaterials) {
	// Asumes normal mesh won't change materials
	m_meshAnim.loadMaterials(fileNames, nrOfMaterials);
}

bool Entity::checkCollision(Entity& other) {
	return m_collisionData.collide(other.m_collisionData);
}

bool Entity::checkCollision(EntityCollision& other) { return m_collisionData.collide(other); }

float Entity::castRay(float3 rayPos, float3 rayDir) {
	if (m_mesh.get() != nullptr) {
		updateMatrix();
		float4x4 mWorld = m_matrixBufferData.matWorld;
		float4x4 mInvWorld = m_matrixBufferData.matWorld.Invert();
		float4x4 mInvWorldInvTra = m_matrixBufferData.matInvTraWorld.Invert();
		// float3 lrayPos = XMVector4Transform(float4(rayPos.x, rayPos.y, rayPos.z, 1), mInvWorld);
		float3 lrayPos = float3::Transform(rayPos, mInvWorld);
		/*float3 lrayDir = XMVector4Transform(
			float4(rayDir.x, rayDir.y, rayDir.z, 0), mInvWorld.Transpose().Invert());*/
		float3 lrayDir = float3::TransformNormal(rayDir, mInvWorldInvTra);
		lrayDir.Normalize();

		float t = m_mesh->castRayOnMesh(lrayPos, lrayDir);
		if (t > 0) {
			float3 target = XMVector3Transform(lrayPos + lrayDir * t, mWorld);
			return (target.x - rayPos.x) / rayDir.x;
		}
		else
			return -1;
	}
	return -1;
}

void Entity::setCollisionData(float3 point, float3 posOffset, float3 scale, float radius) {
	m_collisionData.setCollisionData(point, posOffset, scale, radius);
}

void Entity::setCollisionData(float3 point, float3 posOffset, float3 scale, float3 halfSizes) {
	m_collisionData.setCollisionData(point, posOffset, scale, halfSizes);
}

void Entity::setCollisionDataOBB() {
	if (m_mesh.get() != nullptr)
		setCollisionData(
			getPosition(), m_mesh->getBoundingBoxPos(), m_scale, m_mesh->getBoundingBoxHalfSizes());
	else
		setCollisionData(getPosition(), m_meshAnim.getBoundingBoxPos(), m_scale,
			m_meshAnim.getBoundingBoxHalfSizes());
}

void Entity::setCollisionDataSphere() {
	if (m_mesh.get() != nullptr)
		setCollisionData(getPosition(), m_mesh->getBoundingBoxPos(), m_scale,
			m_mesh->getBoundingBoxHalfSizes().y);
	else
		setCollisionData(getPosition(), m_meshAnim.getBoundingBoxPos(), m_scale,
			m_meshAnim.getBoundingBoxHalfSizes().y);
}

float3 Entity::getHalfSizes() const {
	if (m_mesh.get() != nullptr)
		return m_mesh->getBoundingBoxHalfSizes();
	else
		return m_meshAnim.getBoundingBoxHalfSizes();
}

float3 Entity::getBoundingBoxPos() const {
	if (m_mesh.get() != nullptr)
		return m_mesh->getBoundingBoxPos();
	else
		return m_meshAnim.getBoundingBoxPos();
}

int Entity::getCollisionType() const { return m_collisionData.getCollisionType(); }

float3 Entity::getPointOnOBB(float3 point) const {
	return m_collisionData.getClosestPointOnBox(point);
}

Entity::Entity(string filename, float3 position, float3 scale) {
	load(filename);
	m_position = position;
	m_scale = scale;
	createBuffers();
	m_currentMaterial = 0;
}

Entity::~Entity() {}
