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
			ErrorLogger::logWarning("(Sprite2D) Failed loading sprite: " + paths[failedIndex]);
			return false; // failed if no texture was succesfully loaded
		}
		return true;
	}
	return false;
}

void Sprite2D::set(float2 position, float2 scale, float rotation) { 
	setPosition(position);
	setScale(scale);
	setRotation(rotation);
}

void Sprite2D::_draw(const Transformation2D& source) {
	if (m_textures.size() > 0 && m_textures[0].isLoaded()) {
		size_t texIndex = (size_t)((clock() / 1000.f) / m_animationSpeed) % m_textures.size();

		m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

		float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
		float2 position = source.getPosition() * screenModifier;
		float2 scale = source.getScale() * screenModifier;
		float rotation = source.getRotation();
		float2 alignment =
			float2((float)m_horizontalAligment, (float)m_verticalAlignment) + float2(1, 1);
		float2 texSize =
			float2(m_textures[texIndex].m_textureSize.x, m_textures[texIndex].m_textureSize.y);
		float2 origin = texSize / 2.f * alignment;

		m_spriteBatch->Draw(
			m_textures[texIndex].m_SRV.Get(), position, nullptr, m_color, rotation, origin, scale);

		m_spriteBatch->End();
		// Reset depth state
		Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
	}
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
		return float2((float)texSize.x, (float)texSize.y) * getScale();
	}
	else
		return float2(0, 0);
}

Color Sprite2D::getColor() const { return m_color; }

BoundingBox2D Sprite2D::getBoundingBox() const { 
	//size_t texIndex = (size_t)((clock() / 1000.f) / m_animationSpeed) % m_textures.size();
	float2 position = getPosition();
	float2 size = getSize();
	float2 alignment =
		float2((float)m_horizontalAligment, (float)m_verticalAlignment) + float2(1, 1);
	float2 origin = (size / 2.f) * alignment;
	BoundingBox2D bb(position - origin, position - origin + size); 
	return bb;
}

void Sprite2D::setSize(float2 size) {
	if (m_textures.size() > 0) {
		XMINT2 texSize = getTextureSize();
		setScale(float2(size.x / texSize.x, size.y / texSize.y));
	}
}

void Sprite2D::setAlignment(HorizontalAlignment horizontal, VerticalAlignment vertical) {
	m_horizontalAligment = horizontal;
	m_verticalAlignment = vertical;
}

void Sprite2D::setAnimationSpeed(float animationSpeed) { m_animationSpeed = animationSpeed; }

void Sprite2D::setColor(Color color) { 
	m_color = Color(color.x, color.y, color.z, m_color.w);
}

void Sprite2D::setAlpha(float alpha) { m_color.w = alpha; }

bool Sprite2D::SpriteTexture::load(string path) { 
	m_path = PATH_SPRITE + path;
	wstring str(m_path.begin(), m_path.end());

	m_SRV.Reset();
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	HRESULT res = CreateWICTextureFromFile(Renderer::getDevice(), str.c_str(),
		resource.GetAddressOf(), m_SRV.ReleaseAndGetAddressOf());
	if (FAILED(res)) {
		ErrorLogger::logError("(Sprite2D) Failed to create SRV buffer! Path: " + m_path, res);
		return false;
	}
	else {
		Microsoft::WRL::ComPtr<ID3D11Texture2D> tex;
		resource.As(&tex);
		CD3D11_TEXTURE2D_DESC texDesc;
		tex->GetDesc(&texDesc);

		m_textureSize = XMINT2(texDesc.Width, texDesc.Height);
		m_loaded = true;
		return true;
	}
	resource.Reset();

}

bool Sprite2D::SpriteTexture::isLoaded() const { return m_loaded; }
