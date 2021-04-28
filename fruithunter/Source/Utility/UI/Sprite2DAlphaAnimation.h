#pragma once
#include "Sprite2D.h"
#include "SimpleDirectX.h"

class Sprite2DAlphaAnimation : public Sprite2D {
private:
	static ShaderSet m_shader_alphaAnimation;
	Texture m_sourceTexture, m_sourceBackTexture, m_animationTexture;
	shared_ptr<RenderTexture> m_destinationTexture = make_shared<RenderTexture>();

	struct AnimationSetting {
		float factor = 0;
		int useBackground = false;
		float padding[2];
	} m_animationSetting;
	ConstantBuffer<AnimationSetting> m_cbuffer_animation;

	void _draw(const Transformation2D& source);

	void imgui_animationSetting();

public:
	float getAnimationFactor() const;

	void setAnimationFactor(float factor);

	bool load(string path) = delete;
	bool load(string pathSource, string pathAnimation);
	bool load(string pathSource, string pathSourceBack, string pathAnimation);

	Sprite2DAlphaAnimation();
};
