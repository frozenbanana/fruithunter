#pragma once
#include "Sprite2D.h"
#include <time.h>

class Sprite2DAnimation : public Sprite2D {
private:
	float m_animationSpeed = 1.f; // time between frames, in seconds

	vector<shared_ptr<Texture>> m_textures;

	void _draw(const Transformation2D& source);
	void _imgui_properties();

	size_t getIndex() const;

public:
	bool isLoaded() const;
	void setAnimationSpeed(float animationSpeed);

	bool load(string path) = delete;
	bool load(vector<string> paths, float animationSpeed = 1.f);

};
