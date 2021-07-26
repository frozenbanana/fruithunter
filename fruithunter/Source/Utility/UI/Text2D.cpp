#include "Text2D.h"
#include "Renderer.h"
#include "ErrorLogger.h"

void Text2D::_draw(const Transformation2D& source) {
	ID3D11DepthStencilState* DSS;
	UINT stencilRef;
	Renderer::getDeviceContext()->OMGetDepthStencilState(&DSS, &stencilRef);

	m_spriteBatch->Begin(DirectX::SpriteSortMode_BackToFront);

	float2 screenModifier = float2((SCREEN_WIDTH / 1280.f), (SCREEN_HEIGHT / 720.f));
	std::wstring wText = std::wstring(m_text.begin(), m_text.end());
	float2 alignment(m_horizontalAligment, m_verticalAlignment);
	float2 origin = getLocalSize() * (alignment + float2(1, 1)) * 0.5;
	float2 position = source.getPosition() * screenModifier;
	float2 scale = source.getScale() * screenModifier;
	float rotation = source.getRotation();
	Color color = m_color * m_color.w; // pre multiplied alpha

	// Vector2 origin = Vector2(m_spriteFont->MeasureString(wText.c_str())) / 2.0f;
	m_spriteFont->DrawString(
		m_spriteBatch.get(), wText.c_str(), position, color, rotation, origin, scale);

	m_spriteBatch->End();

	// Reset depth state
	Renderer::getDeviceContext()->OMSetDepthStencilState(DSS, stencilRef);
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

void Text2D::_imgui_properties() {
	ImGui::InputText("Text", &m_text);
	ImGui::ColorEdit4("Color", (float*)&m_color);
}

Color Text2D::getColor() const { return m_color; }

float Text2D::getAlpha() const { return m_color.w; }

Text2D::Text2D() {
	m_spriteBatch = std::make_unique<DirectX::SpriteBatch>(Renderer::getDeviceContext());
	setFont("myfile2.spritefont");
}
