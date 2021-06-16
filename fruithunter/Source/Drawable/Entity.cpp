#include "Entity.h"
#include "Errorlogger.h"
#include "Input.h"
#include "Renderer.h"

bool Entity::isMeshInitialized() const { return (m_mesh.get() != nullptr); }

void Entity::setCollisionForMesh(string meshName) {
	static vector<string> m_treeNames = { "treeMedium1", "treeMedium2", "treeMedium3", "BurnedTree1",
		"BurnedTree2", "BurnedTree3" };
	static vector<string> m_nonCollidables = { "DeadBush", "Grass1", "Grass2", "Grass3", "Grass4" };
	for (size_t i = 0; i < m_treeNames.size(); i++) {
		if (meshName == m_treeNames[i]) {
			setCollidable(true);
			setCollisionDataTree();
			return;
		}
	}
	for (size_t i = 0; i < m_nonCollidables.size(); i++) {
		if (meshName == m_nonCollidables[i]) {
			setCollidable(false);
			return;
		}
	}
	setCollidable(true);
	setCollisionDataOBB();
}

bool Entity::atOrUnder(float terrainHeight) const {
	return getPosition().y <= (terrainHeight + getHalfSizes().y+0.01f);
}

string Entity::getModelName() const {
	if (m_mesh != nullptr) {
		return m_mesh->getName();
	}
	return "";
}

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
	Transformation::setPosition(position);
	m_collisionData.setCollisionPosition(getPosition());
}

void Entity::setRotation(float3 rotation) {
	Transformation::setRotation(rotation);
	m_collisionData.rotateObbAxis(getRotationMatrix());
}

void Entity::setScale(float3 scale) {
	Transformation::setScale(scale);
	m_collisionData.setCollisionScale(getScale());
}

void Entity::setScale(float scale) { setScale(float3(1.) * scale); }

void Entity::isVisible(bool state) { m_visible = state; }

void Entity::draw(float3 color, float alpha) {
	if (m_visible) {
		if (isMeshInitialized()) {
			VSBindMatrix(MATRIX_SLOT);
			setMaterial(m_currentMaterial);
			m_mesh.get()->draw(color, alpha);
		}
	}
}

void Entity::draw_onlyMesh(float3 color, float alpha) {
	if (isMeshInitialized()) {
		VSBindMatrix(MATRIX_SLOT);
		m_mesh.get()->draw_noMaterial(color, alpha);
	}
}

void Entity::draw_boundingBox() {
	if (isMeshInitialized()) {
		VSBindMatrix(MATRIX_SLOT);
		m_mesh.get()->draw_BoundingBox();
	}
}

void Entity::draw_animate(float3 color) {
	VSBindMatrix(MATRIX_SLOT);
	setMaterial(m_currentMaterial);
	m_meshAnim.draw(color);
}

void Entity::draw_animate_onlyMesh(float3 color) {
	VSBindMatrix(MATRIX_SLOT);
	m_meshAnim.draw_onlyMesh(color);
}

void Entity::updateAnimated(float dt) { m_meshAnim.update(dt); }

void Entity::updateAnimatedSpecific(float frameTime) { m_meshAnim.updateSpecific(frameTime); }

void Entity::setFrameTargets(int first, int second) { m_meshAnim.setFrameTargets(first, second); }

bool Entity::load(string filename) {
	shared_ptr<Mesh> m = MeshRepository::get(filename);
	if (m.get() != nullptr) {
		m_mesh = m;
		setCollisionForMesh(getModelName());
		return true;
	}
	else {
		return false;
	}
}

bool Entity::loadAnimated(string filename, int nrOfFrames) {
	if (m_meshAnim.load(filename, nrOfFrames)) {
		setCollisionForMesh(getModelName());
		return true;
	}
	return false;
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
		float4x4 mWorld = getMatrix();
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

bool Entity::castRayEx(float3 rayPos, float3 rayDir, float3& intersection, float3& normal) {
	if (m_mesh.get() != nullptr) {
		float4x4 mWorld = getMatrix();
		float4x4 mInvWorld = mWorld.Invert();
		float4x4 mInvTraWorld = mInvWorld.Transpose();
		float4x4 mInvTraInvWorld = mInvTraWorld.Invert();
		float3 lrayPos = float3::Transform(rayPos, mInvWorld);
		float3 lrayDir = float3::TransformNormal(rayDir, mInvTraInvWorld);
		lrayDir.Normalize();

		float3 cast_target, cast_normal;
		if (m_mesh->castRayOnMeshEx(lrayPos, lrayDir, cast_target, cast_normal)) {
			intersection = XMVector3Transform(cast_target, mWorld);
			normal = XMVector3Transform(cast_normal, mInvTraWorld);
			normal.Normalize();
			return true;
		}
		else
			return false;
	}
	return false;
}

bool Entity::castRayEx_limitDistance(
	float3 point, float3 direction, float3& intersection, float3& normal) {
	if (m_mesh.get() != nullptr) {
		float4x4 mWorld = getMatrix();
		float4x4 mInvWorld = mWorld.Invert();
		float4x4 mInvTraWorld = mInvWorld.Transpose();
		float4x4 mInvTraInvWorld = mInvTraWorld.Invert();
		float3 lrayPos = float3::Transform(point, mInvWorld);
		float3 lrayPosDest = float3::Transform(point + direction, mInvWorld);
		float3 lrayDir = (lrayPosDest - lrayPos);

		float3 cast_target, cast_normal;
		if (m_mesh->castRayOnMeshEx_limitDistance(lrayPos, lrayDir, cast_target, cast_normal)) {
			intersection = XMVector3Transform(cast_target, mWorld);
			normal = XMVector3Transform(cast_normal, mInvTraWorld);
			normal.Normalize();
			return true;
		}
		else
			return false;
	}
	return false;
}

void Entity::setCollisionDataTree() {
	// Scales the OBB down to only encompass the trunk
	if (m_mesh.get() != nullptr) {
		float3 treeScale(0.14f, 1.f, 0.14f);
		setCollisionData(getPosition(), float3(0.f), getScale() * treeScale,
			m_mesh->getBoundingBoxHalfSizes() * treeScale);
	}
	else {
		float3 treeScale(0.14f, 1.f, 0.14f);
		setCollisionData(getPosition(), float3(0.f), getScale() * treeScale,
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
			getPosition(), m_mesh->getBoundingBoxPos(), getScale(), m_mesh->getBoundingBoxHalfSizes());
	else
		setCollisionData(getPosition(), m_meshAnim.getBoundingBoxPos(), getScale(),
			m_meshAnim.getBoundingBoxHalfSizes());
}

void Entity::setCollisionDataSphere() {
	if (m_mesh.get() != nullptr)
		setCollisionData(getPosition(), m_mesh->getBoundingBoxPos(), getScale(),
			m_mesh->getBoundingBoxHalfSizes().Length());
	else
		setCollisionData(getPosition(), m_meshAnim.getBoundingBoxPos(), getScale(),
			m_meshAnim.getBoundingBoxHalfSizes().Length());
}

float3 Entity::getHalfSizes() const {
	if (m_mesh.get() != nullptr)
		return m_mesh->getBoundingBoxHalfSizes() * getScale();
	else
		return m_meshAnim.getBoundingBoxHalfSizes() * getScale();
}

float3 Entity::getBoundingBoxPos() const {
	if (m_mesh.get() != nullptr)
		return m_mesh->getBoundingBoxPos() + getPosition();
	else
		return m_meshAnim.getBoundingBoxPos() + getPosition();
}

int Entity::getCollisionType() const { return m_collisionData.getCollisionType(); }

float3 Entity::getPointOnOBB(float3 point) const {
	return m_collisionData.getClosestPointOnBox(point);
}

bool Entity::getIsCollidable() const { return m_collisionData.getIsCollidable(); }

void Entity::imgui_properties() {
	Transformation::imgui_properties();
	// Mesh
	// Collidable
	bool collidable = m_collisionData.getIsCollidable();
	if (ImGui::Checkbox("Collidable", &collidable)) {
		setCollidable(collidable);
	}
}

Entity::Entity(string filename, float3 position, float3 scale) : Fragment(Fragment::Type::entity) {
	load(filename);
	setPosition(position);
	setScale(scale);
	m_currentMaterial = 0;
}

Entity::~Entity() {}