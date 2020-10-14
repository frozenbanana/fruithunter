#pragma once
#include "GlobalNamespaces.h"
#include <time.h>
#include <WICTextureLoader.h>
#include <SpriteBatch.h>
#include <CommonStates.h>
#include <SpriteFont.h>
#include "Translation2DStructures.h"

#define PATH_SPRITE "assets/sprites/"

enum HorizontalAlignment { hLeft, hCenter, hRight};
enum VerticalAlignment { vTop, vCenter, vBottom };

class Sprite2D {
private:
	static unique_ptr<SpriteBatch> m_spriteBatch;
	static unique_ptr<CommonStates> m_states;
	const string m_prepath = "assets/sprites/";

	struct SpriteTexture {
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
		XMINT2 m_textureSize = XMINT2(0, 0);
		string m_path = "";
		bool load(string path);
	};
	vector<SpriteTexture> m_textures;
	HorizontalAlignment m_horizontalAligment = hLeft;
	VerticalAlignment m_verticalAlignment = vTop;
	float2 m_position = float2(0, 0);
	float2 m_scale = float2(1.f, 1.f);
	float m_rotation = 0.0f;
	float m_animationSpeed = 1.f; // time between frames, in seconds
	Color m_color = Color(1, 1, 1, 1);

public:
	XMINT2 getTextureSize(size_t index = 0) const;
	float2 getSize(size_t index = 0) const;
	float2 getPosition() const;
	float getRotation() const;
	float2 getScale() const;
	Color getColor() const;
	BoundingBox2D getBoundingBox() const;

	void setPosition(float2 position);
	void setScale(float2 scale);
	void setScale(float scale);
	void setSize(float2 size);
	void setRotation(float rot);
	void setAlignment(HorizontalAlignment horizontal = hCenter, VerticalAlignment vertical = vCenter);
	void setAnimationSpeed(float animationSpeed);
	void setColor(Color color);
	void setAlpha(float alpha);

	void draw();
	void drawNoScaling();

	bool load(string path);
	bool load(vector<string> paths, float animationSpeed = 1.f);
	void set(float2 position, float2 scale = float2(1.f, 1.f), float rotation = 0);

	Sprite2D();
	~Sprite2D();
};
