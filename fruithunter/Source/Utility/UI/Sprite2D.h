#pragma once
#include "GlobalNamespaces.h"
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include "Translation2DStructures.h"

#include "Texture.h"

#define PATH_SPRITE "assets/sprites/"

class Sprite2D : public Drawable2D {
private:
	static unique_ptr<SpriteBatch> m_spriteBatch;
	static unique_ptr<CommonStates> m_states;

	Color m_color = Color(1, 1, 1, 1);

protected:
	shared_ptr<Texture> m_texture;

	void giveTexture(shared_ptr<Texture> tex);

	void _draw(const Transformation2D& source);
	void _imgui_properties();

public:
	float2 getLocalSize() const;
	bool isLoaded() const;
	Color getColor() const;
	float getAlpha() const;

	void setSize(float2 size);
	void setColor(Color color);
	void setAlpha(float alpha);

	bool load(string path);
	void set(float2 position, float2 scale = float2(1.f, 1.f), float rotation = 0);

	Sprite2D();
	~Sprite2D();
};
