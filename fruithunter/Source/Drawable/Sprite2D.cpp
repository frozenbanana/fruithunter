#include "Sprite2D.h"
#include "Renderer.h"
#include "ErrorLogger.h"

unique_ptr<SpriteBatch> Sprite2D::m_spriteBatch;
unique_ptr<CommonStates> Sprite2D::m_states;

Sprite2D::Sprite2D() {
	if (m_spriteBatch.get() == nullptr)
		m_spriteBatch = make_unique<SpriteBatch>(Renderer::getDeviceContext());
	if (m_states.get() == nullptr)
		m_states = make_unique<CommonStates>(Renderer::getDevice());
}

Sprite2D::~Sprite2D() {}

void Sprite2D::draw() {
	if (m_textures.size() > 0) {
		size_t texIndex = (size_t)((clock() / 1000.f) / m_animationSpeed) % m_textures.size();

		m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

		float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
		float2 position = m_position * screenModifier;
		float2 scale = m_scale * screenModifier;
		float2 origin = float2(m_textures[texIndex].m_textureSize.x / 2.f,
							m_textures[texIndex].m_textureSize.y / 2.f) *
						float2((float)m_horizontalAligment, (float)m_verticalAlignment);

		m_spriteBatch->Draw(m_textures[texIndex].m_SRV.Get(), position, nullptr, Colors::White,
			m_rotation,
			origin, scale);

		m_spriteBatch->End();
	}
}

void Sprite2D::drawNoScaling() {
	if (m_textures.size() > 0) {
		size_t texIndex = (size_t)((clock() / 1000.f) / m_animationSpeed) % m_textures.size();

		m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

		float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
		float2 position = m_position * screenModifier;
		float2 scale = m_scale;
		float2 origin = float2(m_textures[texIndex].m_textureSize.x / 2.f,
							m_textures[texIndex].m_textureSize.y / 2.f) *
						float2((float)m_horizontalAligment, (float)m_verticalAlignment);

		m_spriteBatch->Draw(m_textures[texIndex].m_SRV.Get(), position, nullptr, Colors::White,
			m_rotation, origin, scale);

		m_spriteBatch->End();
	}
}

bool Sprite2D::load(string path) {
	m_textures.resize(1);
	return m_textures[0].load(path);
}

bool Sprite2D::load(vector<string> paths, float animationSpeed) { 
	if (paths.size() > 0) {
		m_animationSpeed = animationSpeed;
		m_textures.resize(paths.size());
		size_t index = 0;
		size_t failedIndex = -1;
		for (size_t i = 0; i < m_textures.size(); i++) {
			if (m_textures[index].load(paths[i])) {
				index++;
			}
			else 
				failedIndex = i;
		}
		if (index != paths.size())
			m_textures.resize(index);
		if (failedIndex != -1) {
			ErrorLogger::logWarning(HRESULT(), "(Sprite2D) Failed loading sprite: " + paths[failedIndex]);
			return false; // failed if no texture was succesfully loaded
		}
		return true;
	}
	return false;
}

void Sprite2D::set(float2 position, float2 scale, float rotation) { 
	m_position = position;
	m_scale = scale;
	m_rotation = rotation;
}

XMINT2 Sprite2D::getTextureSize(size_t index) const { 
	if (index < m_textures.size() && index >= 0)
		return m_textures[index].m_textureSize;
	else
		return XMINT2(0, 0);
}

float2 Sprite2D::getSize(size_t index) const {
	if (index < m_textures.size() && index >= 0) {
		XMINT2 texSize = getTextureSize(index);
		return float2(texSize.x, texSize.y) * m_scale;
	}
	else
		return float2(0, 0);
}

float2 Sprite2D::getPosition() const { return m_position; }

float Sprite2D::getRotation() const { return m_rotation; }

float2 Sprite2D::getScale() const { return m_scale; }

void Sprite2D::setPosition(float2 position) { m_position = position; }

void Sprite2D::setScale(float2 scale) { m_scale = scale; }

void Sprite2D::setScale(float scale) { m_scale = float2(scale, scale); }

void Sprite2D::setRotation(float rot) { m_rotation = rot; }

void Sprite2D::setAlignment(HorizontalAlignment horizontal, VerticalAlignment vertical) {
	m_horizontalAligment = horizontal;
	m_verticalAlignment = vertical;
}

void Sprite2D::setAnimationSpeed(float animationSpeed) { m_animationSpeed = animationSpeed; }

bool Sprite2D::SpriteTexture::load(string path) { 
	m_path = PATH_SPRITE + path;
	wstring str(m_path.begin(), m_path.end());

	m_SRV.Reset();
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT res = CreateWICTextureFromFile(Renderer::getDevice(), str.c_str(),
		resource.GetAddressOf(), m_SRV.ReleaseAndGetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError(res, "(Sprite2D) Failed to create SRV buffer! Path: " + m_path);
		return false;
	}
	else {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		resource.As(&tex);
		CD3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		m_textureSize = XMINT2(texDesc.Width, texDesc.Height);
		return true;
	}

}
