#pragma once
#include "GlobalNamespaces.h"
#include "Drawable2D.h"
#include <memory>
#include <DirectXMath.h>
#include <SpriteFont.h>


class Text2D : public Drawable2D {
private:
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	std::unique_ptr<SpriteFont> m_spriteFont;

	string m_text;
	Color m_color = Color(1, 1, 1, 1);
	HorizontalAlignment m_horizontal = HorizontalAlignment::Left;
	VerticalAlignment m_vertical = VerticalAlignment::Top;

	void setDepthStateToNull();

	void _draw(const Transformation2D& source);

public:
	float2 getLocalSize() const;
	float2 getSize() const;
	string getText() const;
	Color getColor() const;
	float getAlpha() const;

	void setText(string text);
	void setFont(string font);
	void setColor(Color color);
	void setAlpha(float alpha);
	void setAlignment(HorizontalAlignment ha = Middle, VerticalAlignment va = Center);

	Text2D();

};
