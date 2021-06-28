#include "Sprite2DAnimation.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "TextureRepository.h"

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
			m_textures[index] =
				TextureRepository::get(paths[i], TextureRepository::Type::type_sprites);
			if (m_textures[index].get() == nullptr) {
				ifFailed = true;
			}
			else
				index++;
		}
		if (index != paths.size())
			m_textures.resize(index);
		if (m_textures.size() > 0)
			giveTexture(m_textures[0]);
		return ifFailed;
	}
	return false;
}

void Sprite2DAnimation::_imgui_properties() {
	Sprite2D::_imgui_properties();
	ImGui::Separator();
	for (size_t i = 0; i < m_textures.size(); i++) {
		ImGui::Text(m_textures[i]->getPath().c_str());
	}
}
