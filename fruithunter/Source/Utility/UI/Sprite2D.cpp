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
	m_texture = make_shared<Texture>();
	return m_texture->load(path);
}

void Sprite2D::set(float2 position, float2 scale, float rotation) { 
	setPosition(position);
	setScale(scale);
	setRotation(rotation);
}

void Sprite2D::giveTexture(shared_ptr<Texture> tex) { m_texture = tex; }

void Sprite2D::_draw(const Transformation2D& source) {
	if (isLoaded()) {
		m_spriteBatch->Begin(SpriteSortMode_Deferred, m_states->NonPremultiplied());

		float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
		float2 position = source.getPosition() * screenModifier;
		float2 scale = source.getScale() * screenModifier;
		float rotation = source.getRotation();
		float2 alignment =
			float2((float)m_horizontalAligment, (float)m_verticalAlignment) + float2(1, 1);
		float2 texSize = float2(m_texture->getSize().x, m_texture->getSize().y);
		float2 origin = texSize / 2.f * alignment;

		m_spriteBatch->Draw(
			m_texture->getSRV().Get(), position, nullptr, m_color, rotation, origin, scale);

		m_spriteBatch->End();
		// Reset depth state
		Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
	}
}

bool Sprite2D::isLoaded() const { return m_texture.get() != nullptr && m_texture->isLoaded(); }

XMINT2 Sprite2D::getTextureSize() const { 
	return m_texture->getSize();
}

float2 Sprite2D::getSize() const {
	XMINT2 texSize = getTextureSize();
	return float2((float)texSize.x, (float)texSize.y) * getScale();
}

Color Sprite2D::getColor() const { return m_color; }

HorizontalAlignment Sprite2D::getHorizontalAlignment() const { return m_horizontalAligment; }

VerticalAlignment Sprite2D::getVerticalAlignment() const { return m_verticalAlignment; }

float Sprite2D::getAlpha() const { return m_color.w; }

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
	if (isLoaded()) {
		XMINT2 texSize = getTextureSize();
		setScale(float2(size.x / texSize.x, size.y / texSize.y));
	}
}

void Sprite2D::setAlignment(HorizontalAlignment horizontal, VerticalAlignment vertical) {
	m_horizontalAligment = horizontal;
	m_verticalAlignment = vertical;
}

void Sprite2D::setColor(Color color) { 
	m_color = Color(color.x, color.y, color.z, m_color.w);
}

void Sprite2D::setAlpha(float alpha) { m_color.w = alpha; }
