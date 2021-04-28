#include "Sprite2DAnimation.h"
#include "Renderer.h"
#include "ErrorLogger.h"

void Sprite2DAnimation::_draw(const Transformation2D& source) {
	if (isLoaded()) {
		size_t texIndex = getIndex();
		Sprite2D::giveTexture(m_textures[texIndex]);
		Sprite2D::_draw(source);
	}
}

size_t Sprite2DAnimation::getIndex() const {
	return (size_t)((clock() / 1000.f) / m_animationSpeed) % m_textures.size();
}

bool Sprite2DAnimation::isLoaded() const { 
	if (m_textures.size() == 0)
		return false; // empty
	bool loaded = true;
	for (size_t i = 0; i < m_textures.size(); i++) {
		if (m_textures[i]->isLoaded() == false) {
			loaded = false;
			break;
		}
	}
	return loaded; // contains unloaded sprite
}

XMINT2 Sprite2DAnimation::getTextureSize(size_t index) const {
	if (isLoaded())
		return m_textures[index]->getSize();
	else
		return XMINT2(0, 0);
}

float2 Sprite2DAnimation::getSize(size_t index) const {
	if (isLoaded()) {
		XMINT2 texSize = getTextureSize(index);
		return float2((float)texSize.x, (float)texSize.y) * getScale();
	}
	else
		return float2(0, 0);
}

BoundingBox2D Sprite2DAnimation::getBoundingBox(size_t index) const {
	float2 position = getPosition();
	float2 size = getSize(index);
	float2 alignment =
		float2((float)getHorizontalAlignment(), (float)getVerticalAlignment()) + float2(1, 1);
	float2 origin = (size / 2.f) * alignment;
	BoundingBox2D bb(position - origin, position - origin + size);
	return bb;
}

void Sprite2DAnimation::setSize(float2 size) {
	if (isLoaded()) {
		XMINT2 texSize = getTextureSize(0);
		setScale(float2(size.x / texSize.x, size.y / texSize.y));
	}
}

void Sprite2DAnimation::setAnimationSpeed(float animationSpeed) {
	m_animationSpeed = animationSpeed;
}

bool Sprite2DAnimation::load(vector<string> paths, float animationSpeed) { 
	setAnimationSpeed(animationSpeed);
	if (paths.size() > 0) {
		m_textures.clear();
		m_textures.resize(paths.size());
		size_t index = 0;
		bool ifFailed = false;
		for (size_t i = 0; i < m_textures.size(); i++) {
			if (m_textures[index].get() == nullptr)
				m_textures[index] = make_shared<Texture>();
			if (m_textures[index]->load(paths[i])) {
				index++;
			}
			else {
				ErrorLogger::logWarning(
					"(Sprite2DAnimation) Failed loading sprite: " + paths[i]);
				ifFailed = true;
			}
		}
		if (index != paths.size())
			m_textures.resize(index);
		return ifFailed;
	}
	return false;
}
