#pragma once
#include "Sprite2D.h"
#include <time.h>

class Sprite2DAnimation : public Sprite2D {
private:
	float m_animationSpeed = 1.f; // time between frames, in seconds

	vector<shared_ptr<Texture>> m_textures;

	void _draw(const Transformation2D& source);

	size_t getIndex() const;

public:
	bool isLoaded() const;

	XMINT2 getTextureSize() const = delete;
	float2 getSize() const = delete;
	BoundingBox2D getBoundingBox() const = delete;

	XMINT2 getTextureSize(size_t index = 0) const;
	float2 getSize(size_t index = 0) const;
	BoundingBox2D getBoundingBox(size_t index = 0) const;

	void setSize(float2 size);
	void setAnimationSpeed(float animationSpeed);

	bool load(string path) = delete;
	bool load(vector<string> paths, float animationSpeed = 1.f);

};
