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
		ErrorLogger::logError("Failed to create slider sprite texture", t);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	resource.As(&tex);
	CD3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	m_scale = 0.08f;
	m_radius = texDesc.Width * m_scale * 0.5f;
	m_textureOffset = float2(texDesc.Width / 2.f, texDesc.Height / 2.f);
	m_sliderOffset = float2(150.f, 0.f);

	t = CreateWICTextureFromFile(Renderer::getDevice(), L"assets/sprites/sliderBackground.png",
		resource.GetAddressOf(), m_backgroundTexture.ReleaseAndGetAddressOf());
	if (t)
		ErrorLogger::logError("Failed to create backgorund sprite texture", t);

	resource.As(&tex);
	tex->GetDesc(&texDesc);

	m_backgroundOffset = float2(texDesc.Width / 2.f, texDesc.Height / 2.f);
}

Slider::~Slider() {}

void Slider::initialize(string label, float2 pos) {
	m_label = label;
	m_position = pos;

	m_startPos = pos;
	m_sliderPos = m_startPos;

	m_colour = float4(1.f, 1.f, 1.f, 1.f);
	m_sliding = false;
}

float Slider::getValue() { return m_value; }

void Slider::setPosition(float2 position) {
	float offset = position.x - m_position.x;
	m_position = position;
	m_sliderPos.y = m_position.y;
	m_sliderPos.x += offset;
	m_startPos = m_sliderPos;
}

void Slider::setValue(float value) {
	m_value = value;
	m_sliderPos.x = (m_value * 240.f) + SCREEN_WIDTH / 2 - 120.f;
}

bool Slider::update() {
	Input* ip = Input::getInstance();
	bool changed = false;

	int x = abs(ip->mouseX() - (int)(m_sliderPos.x + m_sliderOffset.x));
	int y = abs(ip->mouseY() - (int)(m_sliderPos.y + m_sliderOffset.y));

	if (x * x + y * y < m_radius * m_radius) {
		m_colour = float4(0.5f, 0.5f, 0.5f, 1.f);
		if (ip->mousePressed(Input::MouseButton::LEFT)) {
			m_grabPos = (float)ip->mouseX();
			m_startPos = m_sliderPos;
			m_offset = 0.f;
			m_sliding = true;
		}
	}
	else if (!m_sliding) {
		m_colour = float4(1.f, 1.f, 1.f, 1.f);
	}

	if (m_sliding && ip->mouseDown(Input::MouseButton::LEFT)) {
		m_offset = ip->mouseX() - m_grabPos;
		m_sliderPos.x =
			max(min(SCREEN_WIDTH / 2 + 120.f, m_startPos.x + m_offset), SCREEN_WIDTH / 2 - 120.f);
	}
	if (m_sliding && ip->mouseReleased(Input::MouseButton::LEFT)) {
		changed = true;
		m_sliding = false;
		m_startPos = m_sliderPos;
		m_value = (m_sliderPos.x - SCREEN_WIDTH / 2 + 120.f) / 240.f;
	}

	return changed;
}

void Slider::draw() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	m_spriteBatch->Draw(m_backgroundTexture.Get(), float2(m_position) + float2(150.f, 0.f), nullptr,
		Colors::White, 0.f, m_backgroundOffset);
	m_spriteBatch->Draw(m_texture.Get(), m_sliderPos + float2(150.f, 0.f), nullptr, m_colour, 0.f,
		m_textureOffset, m_scale);

	m_spriteBatch->End();

	m_textRenderer.draw(m_label + ":", m_position, TextRenderer::Alignment::RIGHT);
}
