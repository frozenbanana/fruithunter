#include "SceneManager.h"
#include "AudioHandler.h"
#include "PathFindingThread.h"
#include "Renderer.h"

shared_ptr<Scene> SceneManager::scene;

void SceneManager::draw_shadow() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<FrustumPlane> planes = shadowMap->getFrustumPlanes();
	for (int i = 0; i < scene->m_fruits.size(); i++) {
		scene->m_fruits[i]->draw_animate_onlyMesh(float3(0, 0, 0));
	}

	// terrain manager
	scene->m_terrains.quadtreeCull(planes);
	scene->m_terrains.draw_onlyMesh();

	// terrain entities
	scene->m_repository.quadtreeCull(planes);
	scene->m_repository.draw_onlyMesh();

	// arrows
	for (size_t i = 0; i < scene->m_arrows.size(); i++)
		scene->m_arrows[i]->draw_onlyMesh(float3(1.));
}

void SceneManager::setup_color(Camera* overrideCamera) {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	Renderer::getInstance()->beginFrame(); // initilize color rendering mode
	shadowMap->setup_shadowRendering();	   // initilize shadows for color rendering mode

	if (overrideCamera == nullptr) {
		scene->m_player->bindMatrix();
	}
	else {
		overrideCamera->bind();
	}

	scene->m_skyBox.bindLightBuffer();
}

void SceneManager::draw_color() {

	// Bow
	scene->m_player->draw();

	// Fruits
	Renderer::getInstance()->enableAlphaBlending();
	for (int i = 0; i < scene->m_fruits.size(); i++) {
		if (scene->m_fruits[i]->isVisible()) {
			scene->m_fruits[i]->draw_animate();
			scene->m_fruits[i]->getParticleSystem()->drawNoAlpha();
		}
	}
	Renderer::getInstance()->disableAlphaBlending();

	// Animals
	for (size_t i = 0; i < scene->m_animals.size(); ++i) {
		scene->m_animals[i].draw();
	}

	// frustum data for culling
	vector<FrustumPlane> frustum = scene->m_player->getFrustumPlanes();
	// Entities
	scene->m_repository.quadtreeCull(frustum);
	scene->m_repository.draw();
	// Terrain
	scene->m_terrains.quadtreeCull(frustum);
	scene->m_terrains.draw();
	// Sea effect
	Renderer::getInstance()->copyDepthToSRV();
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i].quadtreeCull(frustum);
		scene->m_seaEffects[i].draw();
	}

	// SkyBox
	scene->m_skyBox.draw();

	// Dark edges
	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */

	// Particle Systems
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].draw();
	}

	// arrows
	for (size_t i = 0; i < scene->m_arrows.size(); i++)
		scene->m_arrows[i]->draw();

	// arrows particleSystem
	for (size_t i = 0; i < scene->m_arrows.size(); i++)
		scene->m_arrows[i]->draw_trailEffect();
}

void SceneManager::draw_hud() { m_hud.draw(); }

Scene* SceneManager::getScene() { return scene.get(); }

SceneManager::SceneManager() { 
	if(scene.get() == nullptr) 
		scene = make_shared<Scene>(); 
}

SceneManager::~SceneManager() { 
	PathFindingThread::getInstance()->pause(); 
}

void SceneManager::update(Camera* overrideCamera) {
	if (overrideCamera == nullptr)
		m_manualCamera = false;
	else
		m_manualCamera = true;
	//Input::getInstance()->setMouseModeRelative();
	auto pft = PathFindingThread::getInstance();

	scene->m_timer.update();
	float dt = scene->m_timer.getDt();
	Player* player = scene->m_player.get();

	// update player
	if (!m_manualCamera) {
		// reset player (debug purpose)
		if (Input::getInstance()->keyPressed(Keyboard::R))
			scene->resetPlayer();
		player->update(dt);
		// update deltatime if in huntermode
		if (player->inHuntermode()) {
			dt *= 0.1f;
		}
		// drop fruit on key press
		for (int i = 0; i < NR_OF_FRUITS; i++) {
			if (Input::getInstance()->keyPressed(Keyboard::Keys(Keyboard::D1 + i)))
				scene->dropFruit((FruitType)i);
		}
		// Check entity collision
		// player - entity
		for (int i = 0; i < scene->m_repository.getEntities()->size(); i++) {
			player->collideObject(*scene->m_repository.getEntities()->at(i));
		}
	}

	// Update terrainprops
	scene->m_repository.update(dt, player->getCameraPosition(), player->getForward());

	// Update Skybox
	scene->m_skyBox.update(dt);
	const Terrain* activeTerrain = scene->m_terrains.getTerrainFromPosition(player->getPosition());
	if (activeTerrain != nullptr) {
		AreaTag tag = activeTerrain->getTag();
		scene->m_skyBox.switchLight(tag);
		if (!m_manualCamera)
			AudioHandler::getInstance()->changeMusicByTag(tag, dt);
	}

	// update water
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i].update(dt);
	}

	// particle system
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].update(dt);
	}

	// arrows
	for (size_t i = 0; i < scene->m_arrows.size(); i++) {
		if (scene->m_arrows[i]->isActive())
			scene->m_arrows[i]->collide_scene(dt);
		scene->m_arrows[i]->update(dt);
	}

	// for all animals
	for (size_t i = 0; i < scene->m_animals.size(); ++i) {
		Animal* animal = &scene->m_animals[i];
		animal->checkLookedAt(player->getCameraPosition(), player->getForward());
		if (animal->notBribed()) {
			// animal - player
			bool getsThrown = player->checkAnimal(
				animal->getPosition(), animal->getPlayerRange(), animal->getThrowStrength());
			if (getsThrown) {
				animal->makeAngrySound();
				animal->beginWalk(player->getPosition());
			}
			else {
				animal->setAttacked(false);
			}
			// animal - fruits
			for (size_t iFruit = 0; iFruit < scene->m_fruits.size(); ++iFruit) {
				Fruit* fruit = scene->m_fruits[iFruit].get();
				PathFindingThread::lock();
				if (fruit->getFruitType() == animal->getfruitType() &&
					(fruit->getState() == AI::State::RELEASED ||
						fruit->getState() == AI::State::CAUGHT)) {
					float len = (animal->getPosition() - fruit->getPosition()).Length();
					if (len < animal->getFruitRange()) {
						animal->grabFruit(fruit->getPosition());
						scene->m_fruits.erase(scene->m_fruits.begin() + iFruit);
					}
				}
				PathFindingThread::unlock();
			}
		}
		animal->update(dt, player->getPosition());
	}

	// Update fruits, arrow-fruit collision, fruit pickup
	for (int i = 0; i < scene->m_fruits.size(); i++) {
		Fruit* fruit = scene->m_fruits[i].get();

		fruit->getParticleSystem()->update(dt);
		PathFindingThread::lock();
		fruit->update(dt, player->getPosition());
		// collision arrow - fruit
		for (size_t iArrow = 0; iArrow < scene->m_arrows.size(); iArrow++) {
			Arrow* arrow = scene->m_arrows[iArrow].get();
			if (arrow->isActive()) {
				// !! Arrow::collide_entity function doesnt work on Animated Entities.		!!
				// !! Which is the reason i use simpler (and quicker) collision detection.	!!
				if (arrow->checkCollision(*fruit)) {
					// recover stamina
					player->getStaminaBySkillshot(fruit->hit(player->getPosition()));
					// play hit sound
					AudioHandler::getInstance()->playOnceByDistance(
						AudioHandler::HIT_FRUIT, player->getPosition(), fruit->getPosition());
					arrow->collided(arrow->getPosition_front());
				}
			}
		}

		//if (player->isShooting()) {
		//	if (player->getArrow().checkCollision(*fruit)) {
		//		player->getStaminaBySkillshot(fruit->hit(player->getPosition()));
		//		AudioHandler::getInstance()->playOnceByDistance(
		//			AudioHandler::HIT_FRUIT, player->getPosition(), fruit->getPosition());
		//		player->getArrow().setPosition(
		//			float3(-999.f)); // temporary to disable arrow until returning
		//	}
		//}

		// If the fruit is close to the player, then it get picked up
		if (float3(fruit->getPosition() - player->getPosition()).Length() < 1.5f) {
			scene->pickUpFruit(fruit->getFruitType());
			AudioHandler::getInstance()->playOnce(AudioHandler::COLLECT);
			scene->m_fruits.erase(scene->m_fruits.begin() + i);
		}
		PathFindingThread::unlock();
	}
}

void SceneManager::setup_shadow(Camera* overrideCamera) {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<float3> frustumPoints;

	/* -- CONCEPT -- Check frustum edges collision points on terrain to find closest distance to map shadow to*/ 
	//vector<float3> frustumPoints_temp = scene->m_player->getFrustumPoints(1);
	//float min_t = -1;
	//float avg = 0;
	//ImGui::Begin("Rays");
	//for (size_t i = 0; i < 4; i++) {
	//	float t = scene->m_terrains.castRay(
	//		frustumPoints_temp[0], frustumPoints_temp[i+1] - frustumPoints_temp[0]);
	//	if (t == -1)
	//		t = RayPlaneIntersection(frustumPoints_temp[0],
	//			frustumPoints_temp[1] - frustumPoints_temp[0], float3(), float3(0, 1, 0));
	//	t = Clamp<float>(t <= 0 ? 0.4 : t, 0, 0.4);
	//	ImGui::Text("T%i: %f", i,t);
	//	if (t > min_t)
	//		min_t = t;
	//}
	//ImGui::End();

	if (overrideCamera == nullptr)
		frustumPoints = scene->m_player->getFrustumPoints(0.4);
	else
		frustumPoints = overrideCamera->getFrustumPoints(0.4);
	shadowMap->mapShadowToFrustum(frustumPoints); // optimize shadowed area
	shadowMap->setup_depthRendering();			  // initilize depth rendering mode
}

void SceneManager::load(string folder) {
	m_loadedScene = folder;
	//load scene
	PathFindingThread::lock();
	scene->load(folder);
	PathFindingThread::unlock();

	// pathfinding thread
	std::vector<float4> animalPos;
	for (auto a : scene->m_animals)
		animalPos.push_back(
			float4(a.getPosition().x, a.getPosition().y, a.getPosition().z, a.getFruitRange()));
	PathFindingThread::getInstance()->initialize(scene->m_fruits, animalPos);

}

void SceneManager::restart() { load(m_loadedScene); }
