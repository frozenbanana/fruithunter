#include "Editor.h"

void Editor::randomizeProperties(Entity* entity) const {
	entity->setScale(RandomFloat(0.8f, 1.3f));
	entity->rotateY(RandomFloat(0.f, 3.14f * 2.f));
}

bool Editor::tryTreeCollisionData(Entity* entity) {
	for (size_t i = 0; i < m_treeNames.size(); ++i) {
		if (entity->getModelName() == m_treeNames[i]) {
			entity->setCollisionDataTree();
			return true;
		}
	}
	return false;
}

bool Editor::tryNonCollidable(Entity* entity) {
	for (size_t i = 0; i < m_nonCollidables.size(); ++i) {
		if (entity->getModelName() == m_nonCollidables[i]) {
			entity->setCollidable(false);
			return true;
		}
	}
	return false;
}

void Editor::assignCollisionData(Entity* entity) {
	if (tryTreeCollisionData(entity)) {}
	else if (tryNonCollidable(entity)) {
	}
	else {
		entity->setCollisionDataOBB();
	}
}

void Editor::save() {
	
	

}
