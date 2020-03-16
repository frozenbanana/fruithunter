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
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;
	m_startButton.initialize("Start", float2(width / 2, height / 2 - 50));
	m_settingsButton.initialize("Settings", float2(width / 2, height / 2));
	m_exitButton.initialize("Exit", float2(width / 2, height / 2 + 50));

	// Just ignore this. It fixes things.
	m_entity.load("Melon_000000");
	m_entity.setPosition(float3(-1000));

	createLogoSprite();
}

void IntroState::update() { Input::getInstance()->setMouseModeAbsolute(); }

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

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	HRESULT t =
		CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/fruithunter_logo.png",
			resource.GetAddressOf(), m_texture.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError(t, "Failed to create slider sprite texture");

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	resource.As(&tex);
	CD3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	m_scale = 0.3f;
	m_textureOffset = float2(texDesc.Width / 2.f, texDesc.Height / 2.f);

	resource.As(&tex);
	tex->GetDesc(&texDesc);

	// m_backgroundOffset = float2(texDesc.Width / 2.f, texDesc.Height / 2.f);
}

void IntroState::drawLogo() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	// m_spriteBatch->Draw(m_backgroundTexture.Get(), float2(m_position) + float2(150.f, 0.f),
	// nullptr, 	Colors::White, 0.f, m_backgroundOffset);
	m_spriteBatch->Draw(m_texture.Get(), float2(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 6.f), nullptr,
		Colors::White, 0.f, m_textureOffset, m_scale);

	m_spriteBatch->End();
}


void IntroState::play() {
	ErrorLogger::log(m_name + " play() called.");
	float width = SCREEN_WIDTH;
	float height = SCREEN_HEIGHT;

	m_startButton.setPosition(float2(width / 2, height / 2 - 50));
	m_settingsButton.setPosition(float2(width / 2, height / 2));
	m_exitButton.setPosition(float2(width / 2, height / 2 + 50));
}
