#pragma once
#include "GlobalNamespaces.h"
#include <time.h>
#include <WICTextureLoader.h>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include "Drawable2D.h"

#define PATH_SPRITE "assets/sprites/"

class Sprite2D : public Drawable2D {
private:
	static unique_ptr<SpriteBatch> m_spriteBatch;
	static unique_ptr<CommonStates> m_states;

	struct SpriteTexture {
		bool m_loaded = false;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
		XMINT2 m_textureSize = XMINT2(0, 0);
		string m_path = "";
		bool load(string path);
		bool isLoaded() const;
	};
	vector<SpriteTexture> m_textures;
	HorizontalAlignment m_horizontalAligment = AlignLeft;
	VerticalAlignment m_verticalAlignment = AlignTop;
	float m_animationSpeed = 1.f; // time between frames, in seconds
	Color m_color = Color(1, 1, 1, 1);

	void _draw(const Transformation2D& source);

public:
	XMINT2 getTextureSize(size_t index = 0) const;
	float2 getSize(size_t index = 0) const;
	Color getColor() const;
	BoundingBox2D getBoundingBox() const;

	void setSize(float2 size);
	void setAlignment(HorizontalAlignment horizontal = AlignMiddle, VerticalAlignment vertical = AlignCenter);
	void setAnimationSpeed(float animationSpeed);
	void setColor(Color color);
	void setAlpha(float alpha);

	bool load(string path);
	bool load(vector<string> paths, float animationSpeed = 1.f);
	void set(float2 position, float2 scale = float2(1.f, 1.f), float rotation = 0);

	Sprite2D();
	~Sprite2D();
};
