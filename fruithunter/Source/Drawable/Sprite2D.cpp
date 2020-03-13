#include "Sprite2D.h"
#include "Renderer.h"
#include "ErrorLogger.h"

unique_ptr<SpriteBatch> Sprite2D::m_spriteBatch;
unique_ptr<CommonStates> Sprite2D::m_states;

bool Sprite2D::load(string path) {
	m_path = m_prepath + path;
	wstring str(m_path.begin(), m_path.end());

	m_SRV.Reset();
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT res = CreateWICTextureFromFile(Renderer::getDevice(), str.c_str(),
		resource.GetAddressOf(), m_SRV.ReleaseAndGetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError(res, "(LogoLetter) Failed to create logo! Path: " + m_path);
		return false;
	}
	else {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		resource.As(&tex);
		CD3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		m_textureSize = XMINT2(texDesc.Width, texDesc.Height);
		m_initilized = true;
		return true;
	}
}

Sprite2D::Sprite2D() {
	if (m_spriteBatch.get() == nullptr)
		m_spriteBatch = make_unique<SpriteBatch>(Renderer::getDeviceContext());
	if (m_states.get() == nullptr)
		m_states = make_unique<CommonStates>(Renderer::getDevice());
}

Sprite2D::~Sprite2D() {}

void Sprite2D::draw() {
	if (m_initilized) {
		m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

		m_spriteBatch->Draw(m_SRV.Get(), m_position, nullptr, Colors::White, 0.f,
			float2(m_textureSize.x / 2.f, m_textureSize.y / 2.f), m_scale);

		m_spriteBatch->End();
	}
}

XMINT2 Sprite2D::getTextureSize() const { return m_textureSize; }

void Sprite2D::setPosition(float2 position) { m_position = position; }