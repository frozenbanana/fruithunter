#pragma once
#include "GlobalNamespaces.h"
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include "Drawable2D.h"

#include "Texture.h"

#define PATH_SPRITE "assets/sprites/"

class Sprite2D : public Drawable2D {
private:
	static unique_ptr<SpriteBatch> m_spriteBatch;
	static unique_ptr<CommonStates> m_states;

	HorizontalAlignment m_horizontalAligment = AlignLeft;
	VerticalAlignment m_verticalAlignment = AlignTop;
	Color m_color = Color(1, 1, 1, 1);

protected:
	shared_ptr<Texture> m_texture;

	void giveTexture(shared_ptr<Texture> tex);

	void _draw(const Transformation2D& source);

public:
	bool isLoaded() const;
	XMINT2 getTextureSize() const;
	float2 getSize() const;
	BoundingBox2D getBoundingBox() const;

	Color getColor() const;
	HorizontalAlignment getHorizontalAlignment() const;
	VerticalAlignment getVerticalAlignment() const;
	float getAlpha() const;

	void setSize(float2 size);
	void setAlignment(HorizontalAlignment horizontal = AlignMiddle, VerticalAlignment vertical = AlignCenter);
	void setColor(Color color);
	void setAlpha(float alpha);

	bool load(string path);
	void set(float2 position, float2 scale = float2(1.f, 1.f), float rotation = 0);

	Sprite2D();
	~Sprite2D();
};
