#include "EndRoundState.h"
#include "Renderer.h"
#include "Input.h"
#include "AudioHandler.h"
#include "SceneManager.h"
#include "SaveManager.h"

bool EndRoundState::isVictorious() const { return m_time_current < m_time_max; }

void EndRoundState::shootConfetti() { 
	if (m_confettiShot == false) {
		ErrorLogger::log("Confetti shot");
		m_confettiShot = true;
		Color colors[3] = { Color(1, 0, 0, 1), Color(0, 1, 0, 1), Color(0, 0, 1, 1) };
		float2 position(800, 600);
		size_t count = 100;
		m_confetti.clear();
		m_confetti.reserve(count);
		for (size_t i = 0; i < count; i++) {
			float2 size = float2(1, 0.5) * RandomFloat(10, 15);
			float rotationVelocity = RandomFloat(-1, 1) * (3.1415f*2);

			m_confetti.push_back(Confetti(position, size, rotationVelocity, colors[rand() % 3]));
			float rot = -(3.1415f / 2.f) + (rand()%2?-1:1)*pow(RandomFloat(0, 1),1) * (3.1415f / 8);
			m_confetti.back().transform.applyForce(
				float2(cos(rot), sin(rot)) * 6000);
		}
	}
}

EndRoundState::EndRoundState() : StateItem(State::EndRoundState) {}

EndRoundState::~EndRoundState() {}

void EndRoundState::init() {
	analyseScene();

	m_confettiBody.load("square_white.png");

	m_text_pointer.setScale(0.5);
	m_text_header.setScale(1);
	m_text_header.setAlignment(
		TextRenderer::HorizontalAlignment::LEFT, TextRenderer::VerticalAlignment::TOP);
	//background
	m_background.load("ER_background.png");
	m_background.setScale(1.);

	//panel
	m_panel.load("square_white.png");
	m_panel.setPosition(float2(1, 1) * 50);
	m_panel.setSize(float2(1280 - 100, 720 - 100));
	m_panel.setColor(Color(0.49, 0.49, 0.49, 0.9));

	//bar pointer
	m_barPointer.load("square_white.png");
	m_barPointer.setColor(Color(0, 0, 0, 1));
	m_barPointer.setSize(float2(10,30));
	m_barPointer.setAlignment(HorizontalAlignment::hCenter, VerticalAlignment::vTop);

	//bar
	float2 bar_pos = float2(100, 200);
	float2 bar_size_max = float2(m_bar_max, m_bar_height);

	m_bar_back.load("square_white.png");
	m_bar_back.setColor(Color(0.8, 0.8, 1, 1));
	m_bar_back.setPosition(bar_pos);
	m_bar_back.setSize(bar_size_max);

	m_bar_pre.load("square_white.png");
	m_bar_pre.setColor(Color(0, 0, 1, 0.5));
	m_bar_pre.setPosition(bar_pos);
	m_bar_pre.setSize(bar_size_max);

	m_bar_curr.load("square_white.png");
	m_bar_curr.setColor(Color(0, 0, 1, 1));
	m_bar_curr.setPosition(bar_pos);
	m_bar_curr.setSize(bar_size_max);

	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		m_medals[i].setPosition(bar_pos + float2(bar_size_max.x * (m_medals[i].time / m_medals[2].time), 0));
		m_medals[i].setSize(bar_size_max.y*1.5);
	}

	

	//m_victoryText = "Nothing";

	//float width = SCREEN_WIDTH;
	//float height = SCREEN_HEIGHT;
	//m_restartButton.initialize("Restart", float2(width / 2, height / 2 + 50));
	//m_levelSelectButton.initialize("Select Level", float2(width / 2, height / 2 + 120));
	//m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 190));
	//m_particleSystem.load(ParticleSystem::STARS_GOLD, 10);
	//// m_particleSystem.setEmitRate(10.f);
	//m_particleSystem.setPosition(float3(0.0f, -1.f, 0.f));
	//m_timer.reset();
	//m_camera.setView(float3(0.f, 0.f, -1.0f), float3(0.f, 0.f, .0f), float3(0.f, 1.f, .0f));

	//m_background.load("banana.png");
	//m_background.setScale(1.40f);
	//m_background.setRotation(-0.5f);
	//m_background.setPosition(float2((width / 2.5f), (height / 2.0f) - 10.f));

	//m_bowl.load("Bowl");

	//// Just ignore this. It fixes things.
	//m_entity.load("Melon_000000");
	//m_entity.setPosition(float3(-1000));

	//Renderer::getInstance()->captureFrame();

	//string sceneName = SceneManager::getScene()->m_sceneName;
	//const SceneCompletion* savedData = SaveManager::getProgress(sceneName);
	//if (savedData != nullptr) {
	//	switch (savedData->grade) {
	//	case GOLD:
	//		setVictoryText("You earned GOLD");
	//		setVictoryColor(float4(1.0f, 0.85f, 0.0f, 1.0f));
	//		setConfettiPower(22);
	//		setBowl(sceneName, (int)GOLD);
	//		setParticleColorByPrize(GOLD);
	//		break;
	//	case SILVER:
	//		setVictoryText("You earned SILVER");
	//		setVictoryColor(float4(0.8f, 0.8f, 0.8f, 1.0f));
	//		setConfettiPower(18);
	//		setBowl(sceneName, (int)SILVER);
	//		setParticleColorByPrize(SILVER);
	//		break;
	//	case BRONZE:
	//		setVictoryText("You earned BRONZE");
	//		setVictoryColor(float4(0.85f, 0.55f, 0.25f, 1.0f));
	//		setConfettiPower(14);
	//		setBowl(sceneName, (int)BRONZE);
	//		setParticleColorByPrize(BRONZE);
	//		break;
	//	default:
	//		setVictoryText("You earned NOTHING");
	//		setVictoryColor(float4(1.0f, 1.0f, 1.0f, 1.0f));
	//		setConfettiPower(2);
	//		setBowl(sceneName, (int)BRONZE);
	//		setParticleColorByPrize(BRONZE);
	//		break;
	//	}
	//	setTimeText("Time : " + Time2DisplayableString(savedData->timeToComplete));
	//}
}

void EndRoundState::update() {
	Input::getInstance()->setMouseModeAbsolute();
	m_timer.update();
	float dt = m_timer.getDt();

	m_background_offset += m_background_offset_speed * dt;
	while (m_background_offset > 1)
		m_background_offset--;

	// wait 1 second
	float bar_animationTime = 3 * (m_time_current / m_time_max);
	if (m_timer.getTimePassed() > 1) {
		m_bar_timer = Clamp<float>(m_bar_timer + (1.f / bar_animationTime) * dt, 0, 1);

		float animationStage = 1 - pow((m_bar_timer - 1), 2);//equation desribing speed of animation
		m_bar_progress = animationStage * (m_time_current / m_time_max);
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
			float th = m_medals[i].time / m_time_max;
			if (m_bar_progress >= th)
				m_medals[i].activate();
		}
	}

	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++)
		m_medals[i].update(dt);

	//success behavior
	if (m_timer.getTimePassed() > bar_animationTime + 2) {
		m_success_timer = Clamp<float>(m_success_timer + (1/0.5)*dt, 0, 1);
	}

	//confetti
	for (size_t i = 0; i < m_confetti.size(); i++)
		m_confetti[i].update(dt);
	if (m_timer.getTimePassed() > bar_animationTime + 2 + 0.5) {
		shootConfetti();
	}

	//m_timer.update();
	//float dt = m_timer.getDt();
	//m_bowl.rotateY(dt * 0.5f);
	//m_bowlContent.rotateY(dt * 0.5f);
	//m_particleSystem.update(dt);

	//if (m_restartButton.update()) {
	//	SceneManager::getScene()->reset();
	//	pop(true);
	//}
	//if (m_levelSelectButton.update()) {
	//	AudioHandler::getInstance()->pauseAllMusic();
	//	pop(State::LevelSelectState,false);
	//}
	//if (m_exitButton.update()) {
	//	AudioHandler::getInstance()->pauseAllMusic();
	//	pop((State)-1, false);
	//}
}

void EndRoundState::pause() { }

void EndRoundState::play() {
	//Renderer::getInstance()->captureFrame();
	//float width = SCREEN_WIDTH;
	//float height = SCREEN_HEIGHT;

	//// Set the correct bowl
	//// Bowl material and content are set in Playstate handleEvent(hasWon)
	//m_bowl.setPosition(float3(0.0f, 0.0f, 0.0f));
	//m_bowlContent.setPosition(m_bowl.getPosition());
	//m_camera.setEye(m_bowl.getPosition() + float3(0.f, 0.7f, 1.0f));
	//m_camera.setTarget(m_bowl.getPosition() + float3(0.f,0.3f,0.f));

	//m_restartButton.setPosition(float2(width / 2, height / 2 + 50));
	//m_levelSelectButton.setPosition(float2(width / 2, height / 2 + 120));
	//m_exitButton.setPosition(float2(width / 2, height / 2 + 190));

	//AudioHandler::getInstance()->playOnce(AudioHandler::APPLAUSE);
}

void EndRoundState::restart() {}

void EndRoundState::analyseScene() {
	Scene* scene = SceneManager::getScene();
	if (scene == nullptr) {
		// time targets
		const string coin_textures[NR_OF_TIME_TARGETS] = { "coin_gold.png", "coin_silver.png",
			"coin_bronze.png" };
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++)
			m_medals[i].load(coin_textures[i], 30*(i+1));
		// stars
		Color star_colors[NR_OF_TIME_TARGETS] = { Color(1, 1, 0, 1), Color(0.8, 0.8, 0.8, 1),
			Color(0.59, 0.29, 0, 1) };
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
			m_stars[i].load(float2(150, 370) + float2(0, 100 * i), star_colors[i], i * 2);
		}
		// fetch previous time
		m_time_previous = 47;

		m_time_max = m_medals[NR_OF_TIME_TARGETS - 1].time;
		m_time_current = m_time_max;
	}
	else {

		// time targets
		const string coin_textures[NR_OF_TIME_TARGETS] = { "coin_gold.png", "coin_silver.png",
			"coin_bronze.png" };
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++)
			m_medals[i].load(coin_textures[i], scene->m_utility.timeTargets[i]);
		// stars
		Color star_colors[NR_OF_TIME_TARGETS] = { Color(1, 1, 0, 1), Color(0.8, 0.8, 0.8, 1),
			Color(0.59, 0.29, 0, 1) };
		for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
			m_stars[i].load(float2(150, 370) + float2(0, 100 * i), star_colors[i], i * 2);
		}
		// fetch previous time
		const SceneCompletion* progress = SaveManager::getProgress(scene->m_sceneName);
		if (progress) {
			m_time_previous = progress->timeToComplete;
		}
		else {
			m_time_previous = 0;
		}

		m_time_max = m_medals[NR_OF_TIME_TARGETS - 1].time;
		m_time_current = Clamp<float>(scene->m_timer.getTimePassed(), 0, m_time_max);
	}
}

void EndRoundState::draw() {
	//background
	float2 background_size = m_background.getSize();
	int countX = ceil(1280.f / background_size.x)+2;
	int countY = ceil(720.f / background_size.y)+2;
	float2 movement = m_background_offset_direction * background_size;
	for (size_t xx = 0; xx < countX; xx++) {
		for (size_t yy = 0; yy < countY; yy++) {
			m_background.setPosition(movement * m_background_offset +
									 background_size * float2(xx, yy) - background_size);
			m_background.draw();
		}
	}

	//panels
	m_panel.draw();

	//header
	m_text_header.draw("Game Summary", m_panel.getPosition()+float2(1.)*10);

	//stats
	float2 bar_size_max = float2(m_bar_max, m_bar_height);
	m_bar_pre.setSize(float2(bar_size_max.x * (m_time_previous / m_time_max), bar_size_max.y));
	m_bar_curr.setSize(float2(bar_size_max.x * m_bar_progress, bar_size_max.y));

	m_bar_back.draw();
	m_bar_pre.draw();
	m_bar_curr.draw();
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++)
		m_medals[i].draw_back();
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++)
		m_medals[NR_OF_TIME_TARGETS - i - 1].draw_front();

	//bar pointer
	float barPointer_offset = 10;
	m_barPointer.setPosition(m_bar_back.getPosition() + float2(bar_size_max.x * m_bar_progress,
															bar_size_max.y + barPointer_offset));
	m_barPointer.draw();
	string pointerText =
		to_string(int(m_bar_progress * m_time_max)) + (m_bar_progress == 1 ? "+" : "");
	m_text_pointer.draw(
		pointerText, m_barPointer.getPosition() + float2(0, m_barPointer.getSize().y + 10));

	//victory screen
	m_text_success.setScale(m_success_timer*1.5);
	m_text_success.draw((isVictorious()? "Victory" : "Failure"), float2(800, 300));

	//stars
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++)
		m_stars[i].draw();

	//confetti
	for (size_t i = 0; i < m_confetti.size(); i++)
		m_confetti[i].draw(&m_confettiBody);

	//// Draw to shadowmap
	//ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	//shadowMap->mapShadowToFrustum(m_camera.getFrustumPoints(0.1f));
	//shadowMap->setup_depthRendering();
	//m_bowl.draw();
	//m_bowlContent.draw();
	//
	//Renderer::getInstance()->beginFrame();	
	//Renderer::getInstance()->drawCapturedFrame();
	//Renderer::getInstance()->clearDepth();

	//shadowMap->setup_shadowRendering();
	//m_camera.bind();

	//m_particleSystem.draw();
	//m_bowl.draw();
	//m_bowlContent.draw();
	//Renderer::getInstance()->clearDepth();
	//float width = SCREEN_WIDTH;
	//float height = SCREEN_HEIGHT;
	//m_camera.bind();
	////m_background.draw();
	//m_textRenderer.draw(
	//	m_timeText, float2(width / 2, height / 2 - 125), float4(1., 1.f, 1.f, 1.0f));
	//m_textRenderer.draw(m_victoryText, float2(width / 2, height / 2 - 50), m_victoryColor);
	//m_restartButton.draw();
	//m_levelSelectButton.draw();
	//m_exitButton.draw();

}
//
//void EndRoundState::setParticleColorByPrize(size_t prize) {
//	float4 colors[3];
//	switch (prize) {
//	case 0:
//		// gold
//		m_particleSystem.setType(ParticleSystem::STARS_GOLD);
//		break;
//	case 1:
//		// silver
//		m_particleSystem.setType(ParticleSystem::STARS_SILVER);
//		break;
//	default:
//		// bronze
//		m_particleSystem.setType(ParticleSystem::STARS_BRONZE);
//		break;
//	}
//}
//
//void EndRoundState::setBowl(string bowlContentEntityName, int bowlMaterial) {
//	if (bowlContentEntityName == "scene0")
//		m_bowlContent.load("BowlContent1");
//	else if (bowlContentEntityName == "scene1")
//		m_bowlContent.load("BowlContent2");
//	else if (bowlContentEntityName == "scene2")
//		m_bowlContent.load("BowlContent3");
//	m_bowl.setCurrentMaterial((int)bowlMaterial);
//}
//
//void EndRoundState::setTimeText(string text) { m_timeText = text; }
//
//void EndRoundState::setConfettiPower(float emitRate) { m_particleSystem.setEmitRate(emitRate); }
//
//void EndRoundState::setVictoryText(string text) { m_victoryText = text; }
//
//void EndRoundState::setVictoryColor(float4 color) { m_victoryColor = color; }
