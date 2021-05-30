#include "Sprite2DAlphaAnimation.h"
#include "Renderer.h"

ShaderSet Sprite2DAlphaAnimation::m_shader_alphaAnimation;

void Sprite2DAlphaAnimation::_draw(const Transformation2D& source) {
	if (m_sourceTexture.isLoaded()) {
		// imgui_animationSetting();
		// generate
		m_cbuffer_animation.update(m_animationSetting);
		m_cbuffer_animation.bindPS(0);

		m_shader_alphaAnimation.bindShadersAndLayout();

		Renderer::getInstance()->bindQuadVertexBuffer();

		Renderer::getDeviceContext()->PSSetShaderResources(
			0, 1, m_sourceTexture.getSRV().GetAddressOf());
		Renderer::getDeviceContext()->PSSetShaderResources(
			1, 1, m_sourceBackTexture.getSRV().GetAddressOf());
		Renderer::getDeviceContext()->PSSetShaderResources(
			2, 1, m_animationTexture.getSRV().GetAddressOf());

		D3D11_VIEWPORT vp;
		vp.Width = m_destinationTexture->getSize().x;
		vp.Height = m_destinationTexture->getSize().y;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		Renderer::getDeviceContext()->RSSetViewports(1, &vp);

		Renderer::getDeviceContext()->OMSetRenderTargets(
			1, m_destinationTexture->getRTV().GetAddressOf(), nullptr);

		Renderer::getDeviceContext()->Draw(6, 0);
		// Reset
		vp;
		vp.Width = Renderer::getInstance()->getScreenWidth();
		vp.Height = Renderer::getInstance()->getScreenHeight();
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		Renderer::getDeviceContext()->RSSetViewports(1, &vp);
		Renderer::getInstance()->bindRenderAndDepthTarget();


		// supply with texture
		Sprite2D::giveTexture(m_destinationTexture);
		// render
		Sprite2D::_draw(source);
	}
}

void Sprite2DAlphaAnimation::imgui_animationSetting() {
	if (ImGui::Begin("Sprite2DAlphaAnimation - AnimationSetting")) {
		ImGui::SliderFloat("Factor", &m_animationSetting.factor, 0, 1, "%.5f");
	}
	ImGui::End();
}

float Sprite2DAlphaAnimation::getAnimationFactor() const { return m_animationSetting.factor; }

float Sprite2DAlphaAnimation::getFadeLength() const { return m_animationSetting.fadeLength; }

void Sprite2DAlphaAnimation::setAnimationFactor(float factor) {
	m_animationSetting.factor = min(max(factor, 0.f), 1.f);
}

void Sprite2DAlphaAnimation::setFadeLength(float length) { m_animationSetting.fadeLength = length; }

bool Sprite2DAlphaAnimation::load(string pathSource, string pathAnimation) {
	m_animationSetting.useBackground = false;
	if (!m_sourceTexture.load(pathSource))
		return false;
	if (!m_animationTexture.load(pathAnimation))
		return false;
	D3D11_TEXTURE2D_DESC desc;
	m_sourceTexture.getTex2D().Get()->GetDesc(&desc);
	if (m_destinationTexture->create(m_sourceTexture.getSize(), desc.Format))
		return false;
	return true;
}

bool Sprite2DAlphaAnimation::load(string pathSource, string pathSourceBack, string pathAnimation) {
	if (!m_sourceTexture.load(pathSource))
		return false;
	if (!m_sourceBackTexture.load(pathSourceBack))
		return false;
	else
		m_animationSetting.useBackground = true;
	if (!m_animationTexture.load(pathAnimation))
		return false;
	D3D11_TEXTURE2D_DESC desc;
	m_sourceTexture.getTex2D().Get()->GetDesc(&desc);
	if (m_destinationTexture->create(m_sourceTexture.getSize(), desc.Format))
		return false;
	return true;
}

Sprite2DAlphaAnimation::Sprite2DAlphaAnimation() {
	if (!m_shader_alphaAnimation.isLoaded()) {
		D3D11_INPUT_ELEMENT_DESC inputLayout_onlyMesh[] = {
			{
				"Position",					 // "semantic" name in shader
				0,							 // "semantic" index (not used)
				DXGI_FORMAT_R32G32_FLOAT,	 // size of ONE element
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{ "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};

		m_shader_alphaAnimation.createShaders(L"VertexShader_quadSimplePass.hlsl", nullptr,
			L"PixelShader_alphaAnimation.hlsl", inputLayout_onlyMesh, 2);
	}
}
