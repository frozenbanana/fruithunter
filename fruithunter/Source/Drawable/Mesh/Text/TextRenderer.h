#pragma once
#include "GlobalNamespaces.h"
#include "ShaderSet.h"
#include "MeshHandler.h"
#include <memory>
#include <DirectXMath.h>
#include <SpriteFont.h>


class TextRenderer {
public:
	enum HorizontalAlignment { LEFT = -1, MIDDLE = 0, RIGHT = 1 };
	enum VerticalAlignment { TOP = -1, CENTER = 0, BOTTOM = 1 };

	TextRenderer();
	~TextRenderer();
	void setViewSize(XMINT2 size);
	void setFont(string font);

	void draw(string text, float2 pos);
	void drawTextInWorld(string text, float3 position, float3 lookAt, float2 size);

	void setColor(Color color);
	void setScale(float scale);
	void setRotation(float rotation);
	void setAlignment(HorizontalAlignment ha = MIDDLE, VerticalAlignment va = CENTER);

	float2 getSize(string text);

	static float3 normalToRotation(float3 normal);

private:
	Color m_color = Color(1, 1, 1, 1);
	float2 m_alignment = float2(0, 0);
	float m_scale = 1;
	float m_rotation = 0;
	std::unique_ptr<SpriteBatch> m_spriteBatch;
	std::unique_ptr<SpriteFont> m_spriteFont;
	float2 m_fontPos;

	//in world variables
	bool m_initializedViews = false;
	XMINT2 m_size;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
	D3D11_VIEWPORT m_viewport;
	static size_t m_vertexCount;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	struct WorldMatrixData {
		float4x4 mWorld, mInvTraWorld;
	};
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_worldMatrixBuffer;
	static ShaderSet m_shader;

	void createViewBuffers(XMINT2 viewSize);
	void createAndSetVertexBuffer(vector<Vertex> vertices);
	void createMatrixBuffer();
	void createBuffers();

	void updateWorldMatrixBuffer(float3 position, float3 scale, float3 rotation);

	void bindWorldMatrixBuffer();
	void bindVertexBuffer();
	void bindTextView(int slot);

	void setDepthStateToNull();
};
