#include "LevelSelectState.h"
#include "ErrorLogger.h"
#include "Renderer.h"
#include "Statehandler.h"
#include "PlayState.h"
#include "SaveManager.h"
#include "Scene.h"

void LevelSelectState::initialize() {
	m_name = "Level select state";
}

void LevelSelectState::update() {
	Input::getInstance()->setMouseModeRelative();

	sceneManager.update();

	m_timer.update();
	float dt = m_timer.getDt();
	Player* player = SceneManager::getScene()->m_player.get();

	// Update bowls
	for (int i = 0; i < m_levelSelectors.size(); i++) {
		// if index inside array and previous level is completed, then allow to play level
		if (i == 0 || SaveManager::getInstance()->getActiveSave()[i - 1].isCompleted) {
			// Check collision
			vector<shared_ptr<Arrow>> *arrows = &SceneManager::getScene()->m_arrows;
			for (size_t j = 0; j < arrows->size(); j++) {
				if ((*arrows)[j]->isActive() &&
					(*arrows)[j]->collide_entity(dt, m_levelSelectors[i].m_bowl)) {
					// start level
					setLevel(i);
					StateHandler::getInstance()->changeState(StateHandler::PLAY);
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
}

void LevelSelectState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}
	if (Input::getInstance()->keyPressed(Keyboard::M)) {
		StateHandler::getInstance()->changeState(StateHandler::EDITOR);
	}
}

void LevelSelectState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void LevelSelectState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");

	sceneManager.load("levelSelect");

	initializeLevelSelectors();

	//State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	//dynamic_cast<PlayState*>(tempPointer)->destroyLevel(); // reset if there is an old level

	//AudioHandler::getInstance()->changeMusicTo(AudioHandler::ELEVATOR, 0.f); // dt not used. Lazy...
	//State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	//dynamic_cast<PlayState*>(tempPointer)->destroyLevel(); // reset if there is an old level
	//TerrainManager::getInstance()->removeAll();
	//TerrainManager::getInstance()->add(float3(0.f), float3(100.f, 25.f, 100.f), "tutorial.png",
	//	m_maps, XMINT2(210, 210), XMINT2(1, 1), float3(0.f, 0.f, 0.f));

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
		LevelData levelData = SaveManager::getInstance()->getActiveSave()[i];
		size_t minutes = levelData.timeOfCompletion / 60;
		size_t seconds = levelData.timeOfCompletion % 60;
		string strMinutes = "--";
		string strSeconds = "--";
		if (SaveManager::getInstance()->getActiveSave()[i].isCompleted) {
			strMinutes = (minutes < 10 ? "0" : "") + to_string(minutes);
			strSeconds = (seconds < 10 ? "0" : "") + to_string(seconds);
		}
		string str = (levelData.isCompleted ? "COMPLETED" : "");
		str += "\nBest Time: " + strMinutes + "." + strSeconds + " Minutes";
		m_textRenderer.drawTextInWorld(str,
			m_levelSelectors[i].m_bowl.getPosition() + float3(0, 1.5f, 0),
			m_levelSelectors[i].m_bowl.getPosition() + float3(0, 1.5f, 0) -
				SceneManager::getScene()->m_player->getForward(),
			float2(1.f) * 4.f);
	}
	// standard drawing
	sceneManager.draw_color();
	// custom drawing (without darkedges)


}

LevelSelectState::~LevelSelectState() {}

void LevelSelectState::setLevel(int newLevel) {
	State* tempPointer = StateHandler::getInstance()->peekState(StateHandler::PLAY);
	dynamic_cast<PlayState*>(tempPointer)->changeScene("scene"+to_string(newLevel));
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
		bool isCompleted = SaveManager::getInstance()->getActiveSave()[i].isCompleted;
		TimeTargets grade = SaveManager::getInstance()->getActiveSave()[i].grade;
		m_levelSelectors[i].m_bowl.load("Bowl");
		m_levelSelectors[i].m_bowl.setPosition(bowlPositions[i]);
		m_levelSelectors[i].m_bowl.loadMaterials(bowlMaterials);
		if (isCompleted) {
			m_levelSelectors[i].m_bowl.setCurrentMaterial(grade);
			m_levelSelectors[i].m_content.load(bowlContent[i]);
			m_levelSelectors[i].m_content.setPosition(bowlPositions[i]);
		}
		else {
			m_levelSelectors[i].m_bowl.setCurrentMaterial(
				TimeTargets::NR_OF_TIME_TARGETS); // use bowl.mtl material, ignore strange index
		}
	}

	// Initiate animals
	m_animal.clear();
	shared_ptr<Animal> animal = make_shared<Animal>("Bear", 10.f, 7.5f, APPLE, 2, 10.f,
		float3(41.369f, 2.746f, 50.425f), float3(20.f, 3.7f, 90.f), 0.f);
	if (!SaveManager::getInstance()->getActiveSave()[0].isCompleted)
		m_animal.push_back(animal);

	animal = make_shared<Animal>("Goat", 5.f, 3.5f, APPLE, 2, 5.f, float3(52.956f, 2.752f, 65.128f),
		float3(87.f, 8.8f, 156.f), XM_PI * 0.5f);
	if (!SaveManager::getInstance()->getActiveSave()[1].isCompleted)
		m_animal.push_back(animal);
}
