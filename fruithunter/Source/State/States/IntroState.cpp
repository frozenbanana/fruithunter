#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include "TerrainManager.h"

#include "WICTextureLoader.h"



IntroState::IntroState() { initialize(); }

IntroState::~IntroState() {}

void IntroState::initialize() {
	m_name = "Intro State";

	m_startButton.initialize("Start", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 - 50));
	m_settingsButton.initialize("Settings", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2));
	m_exitButton.initialize("Exit", float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));

	// Initialise camera
	// m_camera.setView(float3(61.4f, 16.8f, 44.4f), float3(61.2f, 7.16f, 28.7f), float3(0.f, 1.f,
	// 0.f));
	m_camera.setView(
		//float3(56.4f, 11.0f, 18.2f), float3(68.9f, 10.64f, 23.9f), float3(0.f, 1.f, 0.f));
		float3(58.0f, 10.9f, 21.9f), float3(61.3f, 10.1f, -36.0f), float3(0.f, 1.f, 0.f));

	// Initiate water
	m_waterEffect.initilize(SeaEffect::SeaEffectTypes::water, XMINT2(400, 400), XMINT2(1, 1),
		float3(0.f, 1.f, 0.f) - float3(100.f, 0.f, 100.f), float3(400.f, 2.f, 400.f));

	// Initialise Terrain
	m_maps.push_back("texture_grass.jpg");
	m_maps.push_back("texture_rock6.jpg");
	m_maps.push_back("texture_rock4.jpg");
	m_maps.push_back("texture_rock6.jpg");

	// Initiate terrainProps
	m_terrainProps.load("menu");

	// Initiate apple
	m_apple = make_unique<Apple>(float3(58.0f, 10.1f, 16.9f));
	// m_apple.setPosition(float3(58.f, 11.0f, 18.0f));

	// m_letterPaths[0] = L"assets/sprites/fruithunter_logo2.png";
	m_letterPaths[0] = L"assets/sprites/fruithunter_logo_F.png";
	m_letterPaths[1] = L"assets/sprites/fruithunter_logo_r.png";
	m_letterPaths[2] = L"assets/sprites/fruithunter_logo_u.png";
	m_letterPaths[3] = L"assets/sprites/fruithunter_logo_i.png";
	m_letterPaths[4] = L"assets/sprites/fruithunter_logo_t.png";
	m_letterPaths[5] = L"assets/sprites/fruithunter_logo_H.png";
	m_letterPaths[6] = L"assets/sprites/fruithunter_logo_u.png";
	m_letterPaths[7] = L"assets/sprites/fruithunter_logo_n.png";
	m_letterPaths[8] = L"assets/sprites/fruithunter_logo_t.png";
	m_letterPaths[9] = L"assets/sprites/fruithunter_logo_e.png";
	m_letterPaths[10] = L"assets/sprites/fruithunter_logo_r.png";
	// random seed
	srand((unsigned int)time(NULL));

	createLogoSprite();
	m_timer.reset();
}

void IntroState::update() {

	float3 bowPos(68.9f, 10.64f, 23.9f);
	float3 treePos(56.4f, 9.0f, 18.2f);

	m_timer.update();
	float delta = m_timer.getDt();
	m_totalDelta = fmod((m_totalDelta + delta), (2.f * XM_PI));
	m_shootTime += delta;


	Input::getInstance()->setMouseModeAbsolute();
	m_skybox.updateNewOldLight(2);
	m_skybox.updateCurrentLight();
	m_waterEffect.update(delta);

	m_apple.get()->setPosition(
		float3(treePos.x + (cos(m_totalDelta) * 2.0f), treePos.y, treePos.z + (sin(m_totalDelta) * 2.0f)));

	m_apple.get()->setRotation(float3(0.0f, -m_totalDelta, 0.0f));
	m_apple.get()->update(delta, m_camera.getPosition());

	//float3 bowForward(56.4f - 68.9f, 9.0f - 9.64f, 18.2f - 23.9f);
	float3 bowForward = treePos - bowPos;
	bowForward.Normalize();
	m_bow.charge();
	m_bow.update(delta, bowPos, bowForward,
		bowForward.Cross(float3(0.f, 1.0f, 0.f)),
		TerrainManager::getInstance()->getTerrainFromPosition(bowPos));

	if (m_shootTime > m_shootThreshold) {
		m_shootTime = 0.f;
		m_bow.shoot(float3(bowForward.x, bowForward.y + RandomFloat(0.12f, 0.2f),
						bowForward.z + RandomFloat(0.02f, 0.06f)),
			float3(0.f), -0.003f, 4.224f);
	}
	m_bow.getTrailEffect().update(
		delta, TerrainManager::getInstance()->getTerrainFromPosition(bowPos));
	m_timer.update();
	float t = m_timer.getTimePassed();
	for (size_t i = 0; i < AMOUNT_OF_LETTERS; i++) {

		m_letterPos[i].x += sin(t + m_speedOffsets[i].x) * 0.1f;
		m_letterPos[i].y += cos(t + m_speedOffsets[i].y) * 0.1f;
	}
	Input::getInstance()->setMouseModeAbsolute();
}

	//Arrow collision
	if (!m_bow.getArrowHitObject()) {
		for (int i = 0; i < m_terrainProps.getEntities()->size(); i++) {
			float rayCastingValue = m_terrainProps.getEntities()->at(i)->castRay(
				m_bow.getArrow().getPosition(), m_bow.getArrowVelocity() * delta);
			if (rayCastingValue != -1.f && rayCastingValue < 1.f) {
				// Arrow is hitting object
				float3 target = m_bow.getArrow().getPosition() +
								m_bow.getArrowVelocity() * delta * rayCastingValue;
				m_bow.arrowHitObject(target);
				m_shootThreshold = RandomFloat(2.4f, 4.f);
			}
		}
	}
}

void IntroState::handleEvent() {
	if (m_startButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::LEVEL_SELECT);
	}
	if (m_settingsButton.update()) {
		StateHandler::getInstance()->changeState(StateHandler::SETTINGS);
	}
	if (m_exitButton.update()) {
		StateHandler::getInstance()->quit();
	}
}

void IntroState::pause() {
	ErrorLogger::log(m_name + " pause() called.");
	AudioHandler::getInstance()->pauseAllMusic();
}

void IntroState::draw() {
	// Draw to shadowmap
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	shadowMap->mapShadowToFrustum(m_camera.getFrustumPoints(0.1f));
	shadowMap->setup_depthRendering();

	m_apple.get()->draw_onlyMesh(float3(1.0f));
	m_terrainProps.draw_onlyMesh();
	TerrainManager::getInstance()->draw_onlyMesh();

	// Scene drawing
	Renderer::getInstance()->beginFrame();
	drawLogo();
	shadowMap->setup_shadowsRendering();
	m_skybox.bindLightBuffer();
	m_camera.bindMatrix();
	
	m_bow.draw();
	m_terrainProps.draw();
	TerrainManager::getInstance()->draw();
	Renderer::getInstance()->copyDepthToSRV();
	m_waterEffect.draw();

	Renderer::getInstance()->enableAlphaBlending();
	m_apple.get()->draw_animate();
	Renderer::getInstance()->disableAlphaBlending();

	Renderer::getInstance()->draw_darkEdges();
	m_bow.getTrailEffect().draw();
	m_skybox.draw(2, 2);

	// Draw menu buttons
	m_startButton.draw();
	m_settingsButton.draw();
	m_exitButton.draw();

	// Just ignore this. It fixes things
	m_entity.draw();
}

void IntroState::createLogoSprite() {
	m_spriteBatch = std::make_unique<SpriteBatch>(Renderer::getDeviceContext());
	m_states = std::make_unique<CommonStates>(Renderer::getDevice());

	vector<Microsoft::WRL::ComPtr<ID3D11Resource>> resources;
	resources.resize(AMOUNT_OF_LETTERS);
	for (size_t i = 0; i < AMOUNT_OF_LETTERS; i++) {
		HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), m_letterPaths[i].c_str(),
			resources[i].GetAddressOf(), m_textures[i].ReleaseAndGetAddressOf());
		if (t)
			ErrorLogger::logError(t, "Failed to create logo texture number " + to_string(i));

		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		resources[i].As(&tex);
		CD3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		m_scales[i] = 0.3f;
		m_textureOffsets[i] = float2(texDesc.Width / 2.0f, texDesc.Height / 2.0f);

		resources[i].As(&tex);
		tex->GetDesc(&texDesc);
	}
	float offsetX = STANDARD_WIDTH / 6.f;
	float offsetY = STANDARD_HEIGHT / 6.f;
	for (size_t i = 0; i < AMOUNT_OF_LETTERS; i++) {
		m_speedOffsets[i] = float2(RandomFloat(-0.15f, 0.15f), RandomFloat(-0.5f, 0.5f));
		m_letterPos[i] = float2(offsetX, offsetY);
		offsetX += m_textureOffsets[i].x / 1.55f;
	}
}

void IntroState::drawLogo() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	for (size_t i = 0; i < AMOUNT_OF_LETTERS; i++) {
		m_spriteBatch->Draw(m_textures[i].Get(), m_letterPos[i], nullptr, Colors::White, 0.f,
			m_textureOffsets[i], m_scales[i]);
	}

	m_spriteBatch->End();
}


void IntroState::play() {
	ErrorLogger::log(m_name + " play() called.");

	TerrainManager::getInstance()->removeAll();
	TerrainManager::getInstance()->add(float3(0.f), float3(1.f, 0.10f, 1.f) * 100, "PlainMap.png",
		m_maps, XMINT2(210, 210), XMINT2(1, 1), float3(0.f, 0.f, 0.f));
}
