#include "Text2D.h"
#include "Renderer.h"
#include "ErrorLogger.h"

void Text2D::_draw(const Transformation2D& source) {
	m_spriteBatch->Begin(DirectX::SpriteSortMode_BackToFront);

	float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
	std::wstring wText = std::wstring(m_text.begin(), m_text.end());
	float2 alignment(m_horizontal, m_vertical);
	float2 origin = getLocalSize() * (alignment + float2(1, 1)) * 0.5;
	float2 position = source.getPosition() * screenModifier;
	float2 scale = source.getScale() * screenModifier;
	float rotation = source.getRotation();
	Color color = m_color * m_color.w; // pre multiplied alpha

	// Vector2 origin = Vector2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;
	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), position, color, rotation, origin, scale);

	m_spriteBatch->End();
	setDepthStateToNull();
}

void Text2D::setDepthStateToNull() {
	/*
		spritebatch använder sin egen eller förändrar på något vis dpthstencilstate, vi har
		ingen sånn så sb "slår av" våran depthbuffer, sätt state till nullptr för att slå på vår db
		igen kalla på denna i alla textrenderarens drawfunctioner.
	*/
	Renderer::getInstance()->getDeviceContext()->OMSetDepthStencilState(nullptr, 0);
}

float2 Text2D::getSize() const {
	wstring wText = std::wstring(m_text.begin(), m_text.end());
	return float2(m_spriteFont->MeasureString(wText.c_str()))*getScale();
}

float2 Text2D::getLocalSize() const {
	wstring wText = std::wstring(m_text.begin(), m_text.end());
	return float2(m_spriteFont->MeasureString(wText.c_str()));
}

void Text2D::setFont(string font) {
	string fontPath = "assets/fonts/" + font;
	wstring w_fontPath = wstring(fontPath.begin(), fontPath.end());
	m_spriteFont = std::make_unique<DirectX::SpriteFont>(Renderer::getDevice(), w_fontPath.c_str());

	if (!m_spriteFont.get()) {
		ErrorLogger::log("[Text2D::SetFont] Failed to load font: " + fontPath);
		return;
	}
}

void Text2D::setText(string text) { m_text = text; }

void Text2D::setColor(Color color) {
	m_color.x = color.x;
	m_color.y = color.y;
	m_color.z = color.z;
	// ignore alpha
}

string Text2D::getText() const { return m_text; }

void Text2D::setAlpha(float alpha) { m_color.w = alpha; }

void Text2D::setAlignment(HorizontalAlignment ha, VerticalAlignment va) {
	m_horizontal = ha;
	m_vertical = va;
}

Color Text2D::getColor() const { return m_color; }

float Text2D::getAlpha() const { return m_color.w; }

Text2D::Text2D() {
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Renderer::getDeviceContext());
	setFont("myfile2.spritefont");
}
