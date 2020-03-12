#pragma once
#include "TextRenderer.h"
#include <CommonStates.h>

class Slider {
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;
	float2 m_startPos;
	float2 m_sliderPos;
	float2 m_textureOffset;
	float2 m_backgroundOffset;
	float2 m_sliderOffset;

	float4 m_colour;

	float m_radius;
	float m_scale;

	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<CommonStates> m_states;

	TextRenderer m_textRenderer;
	string m_label;
	float2 m_position;

	float m_grabPos;
	float m_offset;
	float m_value;

	bool m_sliding;

public:
	Slider();
	~Slider();
	void initialize(string label, float2 pos);
	float getValue();
	bool update();
	void draw();
};