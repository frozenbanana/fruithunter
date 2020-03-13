#include "IntroState.h"
#include "ErrorLogger.h"
#include "AudioHandler.h"
#include "Renderer.h"
#include "StateHandler.h"
#include "Input.h"
#include <memory>

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
	m_timer.update();
	float t = m_timer.getTimePassed();
	for (size_t i = 0; i < AMOUNT_OF_LETTERS; i++) {

		m_letterPos[i].x += sin(t + m_speedOffsets[i].x) * 0.1f;
		m_letterPos[i].y += cos(t + m_speedOffsets[i].y) * 0.1f;
	}
	Input::getInstance()->setMouseModeAbsolute();
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
	// Set first person info
	Renderer::getInstance()->beginFrame();
	drawLogo();
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


void IntroState::play() { ErrorLogger::log(m_name + " play() called."); }
