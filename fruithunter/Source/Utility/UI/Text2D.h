#pragma once
#include "GlobalNamespaces.h"
#include "Translation2DStructures.h"
#include <memory>
#include <DirectXMath.h>
#include <SpriteFont.h>


class Text2D : public Drawable2D {
private:
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	std::unique_ptr<SpriteFont> m_spriteFont;

	string m_text;
	Color m_color = Color(1, 1, 1, 1);

protected:
	void _draw(const Transformation2D& source);
	void _imgui_properties();

public:
	float2 getLocalSize() const;
	string getText() const;
	Color getColor() const;
	float getAlpha() const;

	void setText(string text);
	void setFont(string font);
	void setColor(Color color);
	void setAlpha(float alpha);


	Text2D();

};
