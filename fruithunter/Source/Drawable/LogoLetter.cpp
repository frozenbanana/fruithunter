#include "LogoLetter.h"
#include "Renderer.h"
#include "ErrorLogger.h"

unique_ptr<SpriteBatch> LogoLetter::m_spriteBatch;
unique_ptr<CommonStates> LogoLetter::m_states;

void LogoLetter::load(string path) {
	m_letterPath = path;
	wstring str(m_letterPath.begin(), m_letterPath.end());

	Microsoft::WRL::ComPtr<ID3D11Resource> resource;

	HRESULT t = CreateWICTextureFromFile(Renderer::getDevice(), str.c_str(),
		resource.GetAddressOf(), m_SRV.ReleaseAndGetAddressOf());
	if (FAILED(t))
		ErrorLogger::logError(t, "(LogoLetter) Failed to create logo! Path: " + m_letterPath);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
	resource.As(&tex);
	CD3D11_TEXTURE2D_DESC texDesc;
	tex->GetDesc(&texDesc);

	m_textureSize = XMINT2(texDesc.Width, texDesc.Height);
	m_speedOffset = float2(RandomFloat(-0.15f, 0.15f), RandomFloat(-0.5f, 0.5f));
}

LogoLetter::LogoLetter() {
	if (m_spriteBatch.get() == nullptr)
		m_spriteBatch = make_unique<SpriteBatch>(Renderer::getDeviceContext());
	if (m_states.get() == nullptr)
		m_states = make_unique<CommonStates>(Renderer::getDevice());
}

LogoLetter::~LogoLetter() {}

void LogoLetter::draw() {
	m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

	m_spriteBatch->Draw(m_SRV.Get(), m_letterPos, nullptr, Colors::White, 0.f,
		float2(m_textureSize.x / 2.f, m_textureSize.y / 2.f), m_scale);

	m_spriteBatch->End();
}

void LogoLetter::update(float timePassed) {
	m_letterPos =
		float2(sin(timePassed + m_speedOffset.x), cos(timePassed + m_speedOffset.y)) * 0.1f;
}

XMINT2 LogoLetter::getTextureSize() const { return m_textureSize; }

void LogoLetter::setPosition(float2 position) { m_letterPos = position; }