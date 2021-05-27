#include "SceneManager.h"
#include "AudioController.h"
#include "PathFindingThread.h"
#include "Renderer.h"
#include "Settings.h"

shared_ptr<Scene> SceneManager::scene;

void SceneManager::draw_shadow() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<FrustumPlane> planes = shadowMap->getFrustumPlanes();
	for (int i = 0; i < scene->m_fruits.size(); i++) {
		scene->m_fruits[i]->draw_fruit_shadow();
	}

	// terrain manager
	scene->m_terrains.quadtreeCull(planes);
	scene->m_terrains.draw_onlyMesh();

	// terrain entities
	vector<shared_ptr<Entity>*> culledEntities = scene->m_entities.cullElements(planes);
	for (size_t i = 0; i < culledEntities.size(); i++)
		(*culledEntities[i])->draw_onlyMesh(float3(0.));

	// arrows
	for (size_t i = 0; i < scene->m_arrows.size(); i++)
		scene->m_arrows[i]->draw_onlyMesh(float3(1.));
}

void SceneManager::setup_color() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	Renderer::getInstance()->beginFrame(); // initilize color rendering mode
	shadowMap->setup_shadowRendering();	   // initilize shadows for color rendering mode
	
	RPYCamera* camera = &scene->m_camera;
	camera->bind();

	Renderer::getInstance()->setGodRaysSourcePosition(camera->getPosition() + float3(-100, 100, 0));

	scene->m_skyBox.bindLightBuffer();
}

void SceneManager::draw_color() {
	// frustum data for culling
	RPYCamera* camera = &scene->m_camera;
	vector<FrustumPlane> frustum = camera->getFrustumPlanes();
	// Entities
	vector<shared_ptr<Entity>*> culledEntities = scene->m_entities.cullElements(frustum);
	for (size_t i = 0; i < culledEntities.size(); i++)
		(*culledEntities[i])->draw();
	// Terrain
	scene->m_terrains.quadtreeCull(frustum);
	scene->m_terrains.draw();
	// Sea effect
	Renderer::getInstance()->copyDepthToSRV();
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i]->quadtreeCull(frustum);
		scene->m_seaEffects[i]->draw();
	}

	// SkyBox
	scene->m_skyBox.draw();

	// Dark edges
	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */

	// Bow
	if (!m_fotoMode)
		scene->m_player->draw();

	// terrain grass
	scene->m_terrains.draw_grass();

	// arrows entities
	for (size_t i = 0; i < scene->m_arrows.size(); i++)
		scene->m_arrows[i]->draw();

	/* --- Transparent objects --- */

	// Fruits (faces are transparent)
	for (int i = 0; i < scene->m_fruits.size(); i++) {
		scene->m_fruits[i]->draw_fruit();
	}

	// collection points
	for (size_t i = 0; i < scene->m_collectionPoint.size(); i++)
		scene->m_collectionPoint[i]->draw();

	// Particle Systems
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].draw();
	}

	// arrow partile effects
	for (size_t i = 0; i < scene->m_arrowParticles.size(); i++)
		scene->m_arrowParticles[i]->draw();

	Renderer::getInstance()->draw_godRays(camera->getViewProjMatrix());

	//FXAA
	Renderer::getInstance()->draw_FXAA();

	// Capture frame
	Renderer::getInstance()->captureFrame();
}

void SceneManager::draw_hud() {
	if (!m_fotoMode) {
		m_hud.draw();

		float anim = scene->m_player->getBow().getDrawFactor();
		m_crosshair.setScale(anim * 1.f / 35);
		m_crosshair.setAlpha(anim * 0.75f);
		m_crosshair.draw();
	}
}

Scene* SceneManager::getScene() { return scene.get(); }

void SceneManager::setPlayerState(bool state) { m_playerState = state; }

bool SceneManager::getPlayerState() const { return m_playerState; }

void SceneManager::setFotoMode(bool state) { m_fotoMode = state; }

bool SceneManager::getFotoMode() const { return m_fotoMode; }

SceneManager::SceneManager() { 
	if(scene.get() == nullptr) 
		scene = make_shared<Scene>(); 

	m_crosshair.load("crosshair_ingame.png");
	m_crosshair.set(float2(1280. / 2, 720. / 2), float2(1. / 10));
	m_crosshair.setAlignment(); // center - center
}

SceneManager::~SceneManager() { 
	PathFindingThread::getInstance()->pause(); 
}

void SceneManager::update() {

	monitor();

	if (Input::getInstance()->keyPressed(m_key_fotoMode) && DEBUG)
		m_fotoMode = !m_fotoMode;

	//Input::getInstance()->setMouseModeRelative();
	auto pft = PathFindingThread::getInstance();

	scene->m_timer.update();
	float dt = scene->getDeltaTime();
	float dt_nonSlow = scene->getDeltaTime_skipSlow();
	Player* player = scene->m_player.get();
	RPYCamera* camera = &scene->m_camera;

	// update player
	if (m_playerState) {
		player->update();
	}

	// Update Skybox
	scene->m_skyBox.update(dt);

	// update AreaTag
	const Environment* activeEnvironment =
		scene->m_terrains.getTerrainFromPosition(camera->getPosition());
	if (activeEnvironment != nullptr) {
		AreaTag tag = activeEnvironment->getTag();
		scene->update_activeTerrain(tag, m_playerState);
	}

	// update water
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i]->update(dt);
	}

	// particle system
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].update(dt);
	}

	// collection points
	for (size_t i = 0; i < scene->m_collectionPoint.size(); i++) {
		if (scene->m_collectionPoint[i]->update(
				dt, scene->m_player->getPosition() + float3(0, 1.5f / 2, 0))) {
			scene->pickUpFruit(scene->m_collectionPoint[i]->getFruitType());
		}
		if (scene->m_collectionPoint[i]->isFinished()) {
			// remove collection point
			 scene->m_collectionPoint.erase(scene->m_collectionPoint.begin()+i);
			 i--;
		}
	}

	// arrows
	for (size_t i = 0; i < scene->m_arrows.size(); i++) {
		if (scene->m_arrows[i]->isActive())
			scene->m_arrows[i]->collide_scene(dt_nonSlow);
		scene->m_arrows[i]->update(dt_nonSlow);
		if (scene->m_arrows[i]->getPosition().y < 0)
			scene->m_arrows[i]->collided(scene->m_arrows[i]->getPosition()); // stop arrows if below world
	}
	// arrow particles
	for (size_t i = 0; i < scene->m_arrowParticles.size(); i++) {
		scene->m_arrowParticles[i]->update(dt);
		// remove if done (arrow should turn off particle system emiter)
		if (scene->m_arrowParticles[i]->activeParticleCount() == 0) {
			scene->m_arrowParticles.erase(scene->m_arrowParticles.begin()+i);
			i--;
		}
	} 

	// Update fruits, arrow-fruit collision, fruit pickup
	for (int i = 0; i < scene->m_fruits.size(); i++) {
		Fruit* fruit = scene->m_fruits[i].get();

		fruit->getParticleSystem()->update(dt);
		PathFindingThread::lock();
		fruit->update();
		// collision arrow - fruit
		for (size_t iArrow = 0; iArrow < scene->m_arrows.size(); iArrow++) {
			Arrow* arrow = scene->m_arrows[iArrow].get();
			if (arrow->isActive()) {
				// !! Arrow::collide_entity function doesnt work on Animated Entities.		!!
				// !! Which is the reason i use simpler (and quicker) collision detection.	!!
				if (arrow->checkCollision(*fruit)) {
					Skillshot skillshot = Skillshot::SS_BRONZE;
					if (!scene->m_player->inHuntermode()) {
						// recover stamina
						skillshot = fruit->hit(player->getPosition());
						player->getStaminaBySkillshot(skillshot);
					}
					// play hit sound
					SoundID id = AudioController::getInstance()->play("fruit-impact-wet", AudioController::SoundType::Effect);
					AudioController::getInstance()->scaleVolumeByDistance(
						id, (camera->getPosition() - fruit->getPosition()).Length());
					arrow->collided(arrow->getPosition_front());
					// add collection point
					shared_ptr<CollectionPoint> cp = make_shared<CollectionPoint>();
					cp->load(
						fruit->getPosition(), float3(0, 1, 0), fruit->getFruitType(), skillshot);
					scene->m_collectionPoint.push_back(cp);
					// remove fruit
					scene->m_fruits.erase(scene->m_fruits.begin() + i);
					i--;
					//remove arrow
					scene->m_arrows.erase(scene->m_arrows.begin()+iArrow);
					iArrow--;
					break;
				}
			}
		}
		PathFindingThread::unlock();
	}

	m_hud.update(dt);
}

void SceneManager::setup_shadow() {
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<float3> frustumPoints = scene->m_camera.getFrustumPoints(0.4f);

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

	shadowMap->mapShadowToFrustum(frustumPoints); // optimize shadowed area
	shadowMap->setup_depthRendering();			  // initilize depth rendering mode
}

void SceneManager::load(string folder) {
	//load scene
	scene->load(folder);

	// pathfinding thread
	PathFindingThread::getInstance()->initialize(scene->m_fruits);

	m_metricCollector.reset();
}

void SceneManager::reset() { 
	scene->reset(); 
	// pathfinding thread
	PathFindingThread::getInstance()->initialize(scene->m_fruits);

	m_metricCollector.reset();
}

void SceneManager::monitor() {
	if (DEBUG) {
		m_metricCollector.update();
		Input* ip = Input::getInstance();
		if (ip->keyPressed(m_key_monitor))
			m_monitoring = !m_monitoring;
		if (m_monitoring) {
			m_metricCollector.draw_imgui();
		}
	}
}
