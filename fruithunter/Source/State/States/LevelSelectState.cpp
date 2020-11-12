#include "LevelSelectState.h"
#include "SaveManager.h"
#include "Scene.h"
#include "Renderer.h"

void LevelSelectState::init() {
}

void LevelSelectState::update() {
	Input* ip = Input::getInstance();
	if (ip->keyPressed(m_mouseMode_switch))
		m_mouseMode = !m_mouseMode;
	if (m_mouseMode)
		ip->setMouseModeRelative();
	else
		ip->setMouseModeAbsolute();

	sceneManager.update();


	if (ImGui::Begin("TestWindow")) {
		RECT rect;
		GetWindowRect(Renderer::getInstance()->getHandle(), &rect);
		ImGui::Text(string("Width: "+to_string(rect.right-rect.left)).c_str());
		ImGui::Text(string("Height: "+to_string(rect.bottom-rect.top)).c_str());
		ImGui::Text(string("MPosX: " + to_string(Input::getInstance()->mouseX())).c_str());
		ImGui::Text(string("MPosY: " + to_string(Input::getInstance()->mouseY())).c_str());
		ImGui::End();
	}


	m_timer.update();
	float dt = m_timer.getDt();
	Player* player = SceneManager::getScene()->m_player.get();

	// Update bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		// if index inside array and previous level is completed, then allow to play level
		bool available = false;
		if (i == 0)
			available = true;
		else {
			const SceneCompletion* progress = SaveManager::getProgress("scene" + to_string(i - 1));
			if (progress && progress->isCompleted())
				available = true;
		}
		if (available) {
			// Check collision
			vector<shared_ptr<Arrow>> *arrows = &SceneManager::getScene()->m_arrows;
			for (size_t j = 0; j < arrows->size(); j++) {
				if ((*arrows)[j]->isActive() &&
					(*arrows)[j]->collide_entity(dt, m_levelSelectors[i].m_bowl)) {
					// start level
					setLevel(i);
					push(State::PlayState);
				}
			}
		}
	}

	// Update animals
	for (size_t i = 0; i < m_animal.size(); ++i) {
		m_animal[i]->checkLookedAt(player->getCameraPosition(), player->getForward());
		if (m_animal[i]->notBribed()) {
			bool getsThrown = player->checkAnimal(m_animal[i]->getPosition(),
				m_animal[i]->getPlayerRange(), m_animal[i]->getThrowStrength());
			if (getsThrown) {
				m_animal[i]->makeAngrySound();
				m_animal[i]->beginWalk(player->getPosition());
			}
			else {
				m_animal[i]->setAttacked(false);
			}
		}
		m_animal[i]->update(dt, player->getPosition());
	}

	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		push(State::PauseState);
	}
}

void LevelSelectState::draw() {
	//	__SHADOWS__
	sceneManager.setup_shadow();
	sceneManager.draw_shadow();
	// custom drawing (shadow mode)
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		m_levelSelectors[i].m_bowl.draw_onlyMesh(float3(0.));
		m_levelSelectors[i].m_content.draw_onlyMesh(float3(0.));
	}
	// draw animals
	for (int i = 0; i < m_animal.size(); i++) {
		m_animal[i]->draw_onlyAnimal();
	}

	//	__COLOR__
	sceneManager.setup_color();
	// custom color drawing (with darkedges)
	// draw bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		m_levelSelectors[i].m_bowl.draw();
		m_levelSelectors[i].m_content.draw();
	}
	// draw animals
	for (int i = 0; i < m_animal.size(); i++) {
		m_animal[i]->draw_onlyAnimal();
	}
	// text above bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		m_textRenderer.drawTextInWorld(m_levelSelectors[i].timeString,
			m_levelSelectors[i].m_bowl.getPosition() + float3(0, 1.5f, 0),
			m_levelSelectors[i].m_bowl.getPosition() + float3(0, 1.5f, 0) -
				SceneManager::getScene()->m_player->getForward(),
			float2(1.f) * 4.f);
	}
	// standard drawing
	sceneManager.draw_color();
	// custom drawing (without darkedges)


}

void LevelSelectState::play() {
	if (sceneManager.getScene()->m_sceneName != "levelSelect") {
		sceneManager.load("levelSelect");
	}
	initializeLevelSelectors();
}

void LevelSelectState::pause() {}

void LevelSelectState::restart() {}

LevelSelectState::LevelSelectState() : StateItem(State::LevelSelectState) {}

LevelSelectState::~LevelSelectState() {}

void LevelSelectState::setLevel(int newLevel) {
	SceneManager::getScene()->load("scene" + to_string(newLevel));
}

void LevelSelectState::initializeLevelSelectors() {
	// initiate level selectors
	float3 bowlPositions[3]{ float3(7.3f, 2.7f, 47.4f), float3(41.7f, 2.75f, 20.6f),
		float3(90.6f, 2.7f, 47.0f) };
	vector<string> bowlMaterials;
	bowlMaterials.resize(TimeTargets::NR_OF_TIME_TARGETS + 1);
	bowlMaterials[TimeTargets::GOLD] = "BowlGold.mtl";
	bowlMaterials[TimeTargets::SILVER] = "BowlSilver.mtl";
	bowlMaterials[TimeTargets::BRONZE] = "BowlBronze.mtl";
	bowlMaterials[TimeTargets::NR_OF_TIME_TARGETS] = "Bowl.mtl";
	string bowlContent[3] = { "BowlContent1", "BowlContent2", "BowlContent3" };
	m_levelSelectors.resize(3);
	for (size_t i = 0; i < 3; i++) {
		m_levelSelectors[i].m_bowl.load("Bowl");
		m_levelSelectors[i].m_bowl.setPosition(bowlPositions[i]);
		m_levelSelectors[i].m_bowl.loadMaterials(bowlMaterials);
		string strMinutes = "--";
		string strSeconds = "--";

		const SceneCompletion* progress = SaveManager::getProgress("scene"+to_string(i));
		if (progress) {
			size_t minutes = progress->timeToComplete / 60;
			size_t seconds = progress->timeToComplete % 60;
			if (progress->isCompleted()) {
				strMinutes = (minutes < 10 ? "0" : "") + to_string(minutes);
				strSeconds = (seconds < 10 ? "0" : "") + to_string(seconds);
				m_levelSelectors[i].m_bowl.setCurrentMaterial(progress->grade);
				m_levelSelectors[i].m_content.load(bowlContent[i]);
				m_levelSelectors[i].m_content.setPosition(bowlPositions[i]);
				m_levelSelectors[i].timeString =
					"COMPLETED\nBest Time: " + strMinutes + "." + strSeconds + " Minutes";
			}
			else {
				m_levelSelectors[i].m_bowl.setCurrentMaterial(
					TimeTargets::NR_OF_TIME_TARGETS);
				m_levelSelectors[i].timeString =
					"\nBest Time: " + strMinutes + "." + strSeconds + " Minutes";
			}
		}
		else {
			m_levelSelectors[i].timeString =
				"\nBest Time: " + strMinutes + "." + strSeconds + " Minutes";
			m_levelSelectors[i].m_bowl.setCurrentMaterial(TimeTargets::NR_OF_TIME_TARGETS);
		}
	}

	// Initiate animals
	m_animal.clear();
	const SceneCompletion* progress = SaveManager::getProgress("scene0");
	if (progress == nullptr || ( progress && !progress->isCompleted())) {
		shared_ptr<Animal> animal = make_shared<Animal>(float3(41.369f, 2.746f, 50.425f),
			float3(20.f, 3.7f, 90.f), Animal::Type::Bear, FruitType::APPLE, 1, 0);
		m_animal.push_back(animal);
	}
	progress = SaveManager::getProgress("scene1");
	if (progress == nullptr || (progress && !progress->isCompleted())) {
		shared_ptr<Animal> animal = make_shared<Animal>(float3(52.956f, 2.752f, 65.128f),
			float3(87.f, 8.8f, 156.f), Animal::Type::Goat, FruitType::APPLE, 1, XM_PI * 0.5f);
		m_animal.push_back(animal);
	}
}
