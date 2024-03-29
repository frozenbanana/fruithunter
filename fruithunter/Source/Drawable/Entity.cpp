#include "Entity.h"
#include "Errorlogger.h"
#include "Input.h"
#include "Renderer.h"

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

void Entity::lookAt(float3 position) {
	float3 dir = position - m_position;
	dir.Normalize();

	float2 mapY(dir.z, dir.x);
	float2 mapX(float2(dir.x, dir.z).Length(), dir.y);
	mapY.Normalize();
	mapX.Normalize();
	float rotY = (mapY.y >= 0) ? (acos(mapY.x)) : (-acos(mapY.x));
	float rotX = -((mapX.y >= 0) ? (acos(mapX.x)) : (-acos(mapX.x)));
	setRotation(float3(rotX, rotY, 0));
}

void Entity::lookTo(float3 lookAt) {
	float rot = 0.f;
	float dx = lookAt.x - getPosition().x;
	float dz = lookAt.z - getPosition().z;
	if (dx != 0) {
		rot = -atan(dz / dx);
	}
	else {
		rot = 0;
	}

	if (dx < 0) {
		rot = 3.1416f + rot;
	}
	float finalAngle = rot + 3.14f * 0.5f;
	setRotation(float3(0.f, finalAngle, 0.f));
}

void Entity::lookToDir(float3 dir) { lookTo(getPosition() + dir); }

void Entity::draw() {
	if (isMeshInitialized()) {
		bindModelMatrixBuffer();
		setMaterial(m_currentMaterial);
		m_mesh.get()->draw();
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

void Entity::draw_animate_onlyMesh(float3 color) {
	bindModelMatrixBuffer();
	m_meshAnim.draw_onlyMesh(color);
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

void Entity::setCurrentMaterial(int materialIndex) { m_currentMaterial = materialIndex; }

void Entity::setMaterial(int index) {
	if (m_mesh.get() != nullptr)
		return m_mesh->setMaterialIndex(index);
	else
		m_meshAnim.setMaterials(index);
}

void Entity::loadMaterials(std::vector<string> fileNames) {
	if (m_mesh.get() != nullptr)
		return m_mesh->loadOtherMaterials(fileNames);
	else
		m_meshAnim.loadMaterials(fileNames);
}

bool Entity::checkCollision(float3 point) { return m_collisionData.collide(point); }

bool Entity::checkCollision(Entity& other) {
	return m_collisionData.collide(other.m_collisionData);
}

bool Entity::checkCollision(EntityCollision& other) { return m_collisionData.collide(other); }

float Entity::castRay(float3 rayPos, float3 rayDir) {
	if (m_mesh.get() != nullptr) {
		float4x4 mWorld = getModelMatrix();
		float4x4 mInvWorld = mWorld.Invert();
		float4x4 mInvTraWorld = mInvWorld.Transpose();
		float4x4 mInvTraInvWorld = mInvTraWorld.Invert();
		float3 lrayPos = float3::Transform(rayPos, mInvWorld);
		float3 lrayDir = float3::TransformNormal(rayDir, mInvTraInvWorld);
		lrayDir.Normalize();

		float t = m_mesh->castRayOnMesh(lrayPos, lrayDir);
		if (t > 0) {
			float3 target = XMVector3Transform(lrayPos + lrayDir * t, mWorld);
			return (target - rayPos).Length() / rayDir.Length();
		}
		else
			return -1;
	}
	return -1;
}

void Entity::setCollisionDataTree() {
	// Scales the OBB down to only encompass the trunk
	if (m_mesh.get() != nullptr) {
		float3 treeScale(0.14f, 1.f, 0.14f);
		setCollisionData(getPosition(), float3(0.f), m_scale * treeScale,
			m_mesh->getBoundingBoxHalfSizes() * treeScale);
	}
	else {
		float3 treeScale(0.14f, 1.f, 0.14f);
		setCollisionData(getPosition(), float3(0.f), m_scale * treeScale,
			m_meshAnim.getBoundingBoxHalfSizes() * treeScale);
	}
}

void Entity::setCollidable(bool collidable) { m_collisionData.setCollidable(collidable); }

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
			m_mesh->getBoundingBoxHalfSizes().Length());
	else
		setCollisionData(getPosition(), m_meshAnim.getBoundingBoxPos(), m_scale,
			m_meshAnim.getBoundingBoxHalfSizes().Length());
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

bool Entity::getIsCollidable() const { return m_collisionData.getIsCollidable(); }

Entity::Entity(string filename, float3 position, float3 scale) {
	load(filename);
	setPosition(position);
	setScale(scale);
	createBuffers();
	m_currentMaterial = 0;
}

Entity::~Entity() {}
