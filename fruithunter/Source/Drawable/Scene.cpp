#include "Scene.h"
#include "Renderer.h"

void Scene::pushEvent(SceneEvent _event) {
	if (_event != SceneEvent::event_none)
		m_events.push_back(_event);
}

void Scene::pickUpFruit(FruitType fruitType) {
	SceneEvent ev = event_none;
	if (fruitType == FruitType::APPLE)
		ev = event_pickUp_Apple;
	if (fruitType == FruitType::BANANA)
		ev = event_pickUp_Banana;
	if (fruitType == FruitType::MELON)
		ev = event_pickUp_Melon;
	pushEvent(ev);
}

void Scene::dropFruit() {
	// Input* ip = Input::getInstance();
	// auto pft = PathFindingThread::getInstance();

	// if (ip->keyPressed(Keyboard::D1)) {
	//	if (m_inventory[APPLE] >= 0) {
	//		shared_ptr<Apple> apple =
	//			make_shared<Apple>(m_player.getPosition() + float3(0.0f, 1.5f, 0.0f));
	//		apple->release(m_player.getForward());
	//		pft->m_mutex.lock();
	//		m_fruits.push_back(apple);
	//		pft->m_mutex.unlock();

	//		// m_inventory[APPLE]--;
	//		m_hud.removeFruit(APPLE);
	//	}
	//}
	// if (ip->keyPressed(Keyboard::D2)) {
	//	if (m_inventory[BANANA] >= 0) {
	//		shared_ptr<Banana> banana =
	//			make_shared<Banana>(float3(m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
	//		banana->release(m_player.getForward());
	//		pft->m_mutex.lock();
	//		m_fruits.push_back(banana);
	//		pft->m_mutex.unlock();
	//		// m_inventory[BANANA]--;
	//		m_hud.removeFruit(BANANA);
	//	}
	//}
	// if (ip->keyPressed(Keyboard::D3)) {
	//	if (m_inventory[MELON] >= 0) {
	//		shared_ptr<Melon> melon =
	//			make_shared<Melon>((m_player.getPosition() + float3(0.0f, 1.5f, 0.0f)));
	//		melon->release(m_player.getForward());
	//		pft->m_mutex.lock();
	//		m_fruits.push_back(melon);
	//		pft->m_mutex.unlock();
	//		// m_inventory[MELON]--;
	//		m_hud.removeFruit(MELON);
	//	}
	//}
}

SceneEvent Scene::popEvent() {
	if (m_events.size() > 0) {
		SceneEvent ev = m_events.back();
		m_events.pop_back();
		return ev;
	}
	return SceneEvent::event_none;
}

void Scene::restart() { m_player.setPosition(spawnPoint); }

void Scene::update(float dt) {
	auto pft = PathFindingThread::getInstance();

	// update player and players particle system
	m_player.update(dt, m_terrains.getTerrainFromPosition(m_player.getPosition()));
	m_player.getBow().getTrailEffect().update(dt);

	// global slow motion
	if (m_player.inHuntermode()) {
		dt *= 0.1f;
	}

	// for all animals
	for (size_t i = 0; i < m_animals.size(); ++i) {
		m_animals[i]->checkLookedAt(m_player.getCameraPosition(), m_player.getForward());
		if (m_animals[i]->notBribed()) {
			bool getsThrown = m_player.checkAnimal(m_animals[i]->getPosition(),
				m_animals[i]->getPlayerRange(), m_animals[i]->getThrowStrength());
			if (getsThrown) {
				m_animals[i]->makeAngrySound();
				m_animals[i]->beginWalk(m_player.getPosition());
			}
			else {
				m_animals[i]->setAttacked(false);
			}


			for (size_t iFruit = 0; iFruit < m_fruits.size(); ++iFruit) {
				pft->m_mutex.lock();
				if (m_fruits[iFruit]->getFruitType() == m_animals[i]->getfruitType()) {
					float len =
						(m_animals[i]->getPosition() - m_fruits[iFruit]->getPosition()).Length();
					if (len < m_animals[i]->getFruitRange()) {
						m_animals[i]->grabFruit(m_fruits[iFruit]->getPosition());
						m_fruits.erase(m_fruits.begin() + iFruit);
					}
				}
				pft->m_mutex.unlock();
			}
		}
		m_animals[i]->update(dt, m_player.getPosition());
	}
	dropFruit();

	// update skybox
	m_skybox.updateDelta(dt);
	AreaTags activeTag = m_terrains.getTerrainFromPosition(m_player.getPosition())->getTag();
	if (m_skybox.updateNewOldLight(activeTag)) {
		AudioHandler::getInstance()->changeMusicByTag(activeTag, dt);
	}
	m_skybox.updateCurrentLight();

	// update fruits
	for (int i = 0; i < m_fruits.size(); i++) {
		// m_fruits[i]->getParticleSystem()->update(dt);// done in fuit update now
		pft->m_mutex.lock();
		m_fruits[i]->update(dt, m_player.getPosition());
		if (m_player.isShooting()) {
			if (m_player.getArrow().checkCollision(*m_fruits[i])) {
				m_fruits[i]->hit(m_player.getPosition());
				AudioHandler::getInstance()->playOnceByDistance(
					AudioHandler::HIT_FRUIT, m_player.getPosition(), m_fruits[i]->getPosition());

				m_player.getArrow().setPosition(
					float3(-999.f)); // temporary to disable arrow until returning
			}
		}
		if (float3(m_fruits[i].get()->getPosition() - m_player.getPosition()).Length() <
			1.5f) { // If the fruit is close to the player get picked up
			pickUpFruit(m_fruits[i].get()->getFruitType());
			AudioHandler::getInstance()->playOnce(AudioHandler::COLLECT);
			m_fruits.erase(m_fruits.begin() + i);
		}
		pft->m_mutex.unlock();
	}

	// Check entity collisions
	// player - entity
	for (size_t iObj = 0; iObj < m_entities.size(); ++iObj) {
		m_player.collideObject(*m_entities[iObj].get());
	}

	// Check entity - arrow
	float3 arrowPosision = m_player.getArrow().getPosition();
	float3 arrowVelocity = m_player.getBow().getArrowVelocity();
	vector<shared_ptr<Entity>*> entitiesAroundArrow =
		m_entities.getElementsByPosition(arrowPosision);
	if (m_player.isShooting() && !m_player.getBow().getArrowHitObject()) {
		for (size_t i = 0; i < entitiesAroundArrow.size(); i++) {
			if ((*entitiesAroundArrow[i])->getIsCollidable()) {
				float castray =
					(*entitiesAroundArrow[i])->castRay(arrowPosision, arrowVelocity * dt);
				if (castray != -1.f && castray < 1.f) {
					// Arrow is hitting object
					float3 target = arrowPosision + arrowVelocity * dt * castray;
					m_player.getBow().arrowHitObject(target);
				}
			}
		}
	}

	// update particle effects
	for (size_t i = 0; i < m_particleEffects.size(); i++) {
		Terrain* currentTerrain =
			m_terrains.getTerrainFromPosition(m_particleEffects[i]->getPosition());
		if (currentTerrain != nullptr) {
			if (m_particleEffects[i]->getType() == ParticleSystem::VULCANO_SMOKE ||
				m_particleEffects[i]->getType() == ParticleSystem::VULCANO_FIRE ||
				m_particleEffects[i]->getType() == ParticleSystem::LAVA_BUBBLE) {
				m_particleEffects[i]->update(dt, currentTerrain->getWindStatic());
			}
			else {
				m_particleEffects[i]->update(dt, currentTerrain); // Get wind dynamically
			}
		}
	}

	// update sea effect
	for (size_t i = 0; i < m_seaEffects.size(); i++) {
		m_seaEffects[i]->update(dt);
	}
}

void Scene::draw() {
	m_skybox.bindLightBuffer();
	m_player.draw();

	// draw fruits
	Renderer::getInstance()->enableAlphaBlending();
	for (int i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->draw_animate();
	}
	Renderer::getInstance()->disableAlphaBlending();

	// draw animals
	for (size_t i = 0; i < m_animals.size(); ++i) {
		m_animals[i]->draw();
	}

	// frustum data for culling
	vector<FrustumPlane> frustum = m_player.getFrustumPlanes();
	// draw entities
	vector<shared_ptr<Entity>*> culledEntities = m_entities.cullElements(frustum);
	for (size_t i = 0; i < culledEntities.size(); i++) {
		(*culledEntities[i])->draw();
	}

	// draw terrain
	m_terrains.quadtreeCull(frustum);
	m_terrains.draw();

	// draw sea effects
	Renderer::getInstance()->copyDepthToSRV();
	for (size_t i = 0; i < m_seaEffects.size(); i++) {
		m_seaEffects[i]->quadtreeCull(frustum);
		m_seaEffects[i]->draw();
	}

	// dark edges
	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */
	// Particle Systems
	for (size_t i = 0; i < m_particleEffects.size(); i++) {
		m_particleEffects[i]->draw();
	}
	// draw fruit particleSystem
	for (size_t i = 0; i < m_fruits.size(); i++) {
		m_fruits[i]->getParticleSystem()->drawNoAlpha();
	}
	// draw player particle system
	m_player.getBow().getTrailEffect().draw();

	// draw skybox
	m_skybox.draw();
}

void Scene::clear() {
	m_terrains.removeAll();
	m_entities.reset();
	m_seaEffects.clear();
	m_fruits.clear();
	m_bowls.clear();
	m_particleEffects.clear();
	m_animals.clear();
	m_events.clear();
}

void Scene::loadTEPS(string filename) {
	struct EntityInstance {
		float3 position, scale;
		float4x4 matRotation;
	};
	// reset repository to fill with new stuff
	m_entities.reset();
	// load repository
	fstream file;
	string path = m_entityPlacementFilePath + filename + m_fileEndings;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		// read mesh count
		int meshCount = 0;
		file.read((char*)&meshCount, sizeof(meshCount));
		for (size_t m = 0; m < meshCount; m++) {
			string meshName = "";
			// read mesh name
			char* str = nullptr;
			int meshNameCount = 0;
			file.read((char*)&meshNameCount, sizeof(meshNameCount));
			str = new char[meshNameCount];
			file.read(str, meshNameCount);
			for (size_t i = 0; i < meshNameCount; i++)
				meshName += str[i]; // set name
			delete[] str;
			// read mesh instance count
			int instanceCount = 0;
			file.read((char*)&instanceCount, sizeof(instanceCount));
			m_entities.reserve(m_entities.size() + instanceCount);
			// read instances
			EntityInstance instance;
			for (size_t i = 0; i < instanceCount; i++) {
				// read instance
				file.read((char*)&instance, sizeof(EntityInstance));
				// inser
				shared_ptr<Entity> entity =
					make_shared<Entity>(meshName, instance.position, instance.scale);
				entity->setRotationMatrix(instance.matRotation);
				m_entities.add(entity->getLocalBoundingBoxPosition(),
					entity->getLocalBoundingBoxSize(), entity->getModelMatrix(), entity);
			}
		}
		file.close();
		ErrorLogger::log("(Scene) Loaded entity placements from path: " + path);
	}
	else {
		ErrorLogger::logWarning(HRESULT(), "(Scene) Couldnt open and load from path: " + path);
	}
}

void Scene::load(string sceneName) {
	if (sceneName == "") {
		// do nothing
	}
	else {
		clear();
		if (sceneName == "level0") {}
		else if (sceneName == "level1") {
		}
		else if (sceneName == "level2") {
		}
		else if (sceneName == "levelSelector") {
			spawnPoint = float3(34.0f, 2.5f, 79.9f);
			vector<string> terrainMaps;
			terrainMaps.push_back("texture_grass.jpg");
			terrainMaps.push_back("texture_rock6.jpg");
			terrainMaps.push_back("texture_rock4.jpg");
			terrainMaps.push_back("texture_rock6.jpg");
			// Initiate animals
			shared_ptr<Animal> animal = make_shared<Animal>("Bear", 10.f, 7.5f, APPLE, 2, 10.f,
				float3(41.369f, 2.746f, 50.425f), float3(20.f, 3.7f, 90.f), 0.f);
			m_animals.push_back(animal);

			animal = make_shared<Animal>("Goat", 5.f, 3.5f, APPLE, 2, 5.f,
				float3(52.956f, 2.752f, 65.128f), float3(87.f, 8.8f, 156.f), XM_PI * 0.5f);
			m_animals.push_back(animal);
			// Initiate water
			shared_ptr<SeaEffect> sea = make_shared<SeaEffect>();
			sea->initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
				float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));
			m_seaEffects.push_back(sea);
			loadTEPS("levelSelection");
			// initiate level selectors
			m_bowls.resize(3);
			for (size_t i = 0; i < 3; i++) {
				m_bowls[i] = make_unique<Entity>();
				m_bowls[i]->load("Bowl");
			}
			m_bowls[0]->setPosition(float3(7.3f, 3.0f, 47.4f));
			m_bowls[1]->setPosition(float3(41.7f, 3.0f, 20.6f));
			m_bowls[2]->setPosition(float3(90.6f, 3.0f, 47.0f));
		}
		else {
			// do nothing
		}
	}
}

Scene::Scene() {}

Scene::~Scene() {}
