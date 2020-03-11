#include "Slider.h"
#include "Renderer.h"
#include "WICTextureLoader.h"
#include "ErrorLogger.h"
#include "Input.h"

Slider::Slider() {
	m_spriteBatch = std::make_unique<SpriteBatch>(Renderer::getDeviceContext());
	m_states = std::make_unique<CommonStates>(Renderer::getDevice());

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/apple.png",
		resource.GetAddressOf(), m_texture.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError(t, "Failed to create slider sprite texture");
	m_startPos = float2(STANDARD_WIDTH / 2, STANDARD_HEIGHT - 150.0f);
	m_pos = m_startPos;

	t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/sliderBackground.png",
		resource.GetAddressOf(), m_backgroundTexture.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError(t, "Failed to create backgorund sprite texture");
}

Slider::~Slider() {}

float Slider::update() {
	Input* input = Input::getInstance();
	if (input->mousePressed(Input::MouseButton::LEFT)) {
		m_grabPos = input->mouseX();
		m_offset = 0.f;
	}
	if (input->mouseDown(Input::MouseButton::LEFT)) {
		m_offset = input->mouseX() - m_grabPos;
		m_pos.x = max(
			min(STANDARD_WIDTH / 2 + 200.f, m_startPos.x + m_offset), STANDARD_WIDTH / 2 - 200.f);
	}
	if (input->mouseReleased(Input::MouseButton::LEFT)) {
		m_startPos = m_pos;
	}

	return 0.0f;
}

void Slider::draw() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	m_spriteBatch->Draw(m_backgroundTexture.Get(),
		float2(STANDARD_WIDTH / 2 - 150.f, STANDARD_HEIGHT - 150.f), nullptr, Colors::White, 0.f,
		float2(0.f), 1.0f);
	m_spriteBatch->Draw(m_texture.Get(), m_pos, nullptr, Colors::White, 0.f, float2(0.f), 0.2f);

	m_spriteBatch->End();
}
