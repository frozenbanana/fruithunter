#pragma once
#include "TextRenderer.h"
#include <CommonStates.h>

class Slider {
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_texture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_backgroundTexture;
	float2 m_startPos;
	float2 m_pos;

	unique_ptr<SpriteBatch> m_spriteBatch;
	unique_ptr<CommonStates> m_states;

	TextRenderer m_textRenderer;
	string m_label;
	float2 m_position;
	float2 m_size;
	float4 m_colour;

	float m_grabPos;
	float m_offset;

public:
	Slider();
	~Slider();
	float update();
	void draw();
};