#include "Sprite2D.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "filesystemHelper.h"
#include "TextureRepository.h"

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
	m_texture = TextureRepository::get(path, TextureRepository::Type::type_sprites);
	return isLoaded();
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

float2 Sprite2D::getLocalSize() const {
	if (m_texture.get() == nullptr)
		return float2();
	return float2(m_texture->getSize().x, m_texture->getSize().y);
}

Color Sprite2D::getColor() const { return m_color; }

float Sprite2D::getAlpha() const { return m_color.w; }

void Sprite2D::setSize(float2 size) {
	if (isLoaded()) {
		float2 lsize = getLocalSize();
		setScale(size / lsize);
	}
}

void Sprite2D::setColor(Color color) { m_color = Color(color.x, color.y, color.z, m_color.w); }

void Sprite2D::setAlpha(float alpha) { m_color.w = alpha; }

void Sprite2D::_imgui_properties() {
	static bool texInit = false;
	static vector<shared_ptr<Texture>> textures;
	if (!texInit) {
		texInit = true;
		vector<string> textureStr;
		SimpleFilesystem::readDirectory(PATH_SPRITE, textureStr);
		textures.resize(textureStr.size());
		for (size_t i = 0; i < textureStr.size(); i++)
			textures[i] =
				TextureRepository::get(textureStr[i], TextureRepository::Type::type_sprites);
	}
	if (ImGui::BeginCombo(
			"Texture", m_texture.get() != nullptr ? m_texture->getFilename().c_str() : "")) {
		for (size_t i = 0; i < textures.size(); i++) {
			ImGui::BeginGroup();
			ImGui::Text(textures[i]->getFilename().c_str());
			ImVec2 texsize = ImVec2(textures[i]->getSize().x, textures[i]->getSize().y);
			float smax = max(texsize.x, texsize.y);
			ImVec2 size = (texsize / smax) * 100.f;
			if (ImGui::ImageButton(textures[i]->getSRV().Get(), size)) {
				m_texture = textures[i];
			}
			ImGui::EndGroup();
			if (m_texture.get() == textures[i].get())
				ImGui::SetItemDefaultFocus();
			if ((i + 1) % 3)
				ImGui::SameLine();
		}
		ImGui::EndCombo();
	}
	ImGui::ColorEdit4("Color", (float*)&m_color);
}