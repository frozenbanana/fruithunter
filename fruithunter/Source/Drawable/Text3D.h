#pragma once
#include "GlobalNamespaces.h"
#include "Transformation.h"
#include "MeshHandler.h"
#include "ShaderSet.h"
#include "Text2D.h"

class Text3D : public Transformation, private Text2D {
private:
	bool m_initializedViews = false;
	XMINT2 m_size;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SRV;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RTV;
	D3D11_VIEWPORT m_viewport;
	static size_t m_vertexCount;
	static Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	static ShaderSet m_shader;

	void createViewBuffers(XMINT2 viewSize);
	void createVertexBuffer(const vector<Vertex>& vertices);
	void createBuffers();

	void bindVertexBuffer();
	void bindTextView(int slot);

public:
	void setTarget(float3 target);
	void setText(string text);
	void setFont(string font);
	void setColor(Color color);

	void draw();

	Text3D();

};
