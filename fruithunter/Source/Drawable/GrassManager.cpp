#include "GrassManager.h"
#include "ErrorLogger.h"
#include "SceneManager.h"

ShaderSet GrassManager::m_shader;

void GrassManager::GrassPatch::createBuffer() {
	if (m_straws.size() > 0) {
		// vertex buffer
		m_vertexBuffer.Reset();
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = (UINT)m_straws.size() * sizeof(Straw);
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_straws.data();
		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&bufferDesc, &data, m_vertexBuffer.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(
				"Failed creating vertex buffer in GrassManager::GrassPatch class!\n", res);
	}
}

void GrassManager::GrassPatch::generate(
	float2 position, float2 size, size_t count, Terrain& terrain) {
	if (count > 0) {
		size_t nrOfTries = 0;
		size_t maxTries = Clamp<int>(count * 0.15f, 1, count);
		bool earlyQuitCheck = true;
		Straw straw;
		m_straws.reserve(count);
		for (size_t i = 0; i < count; i++) {
			if (earlyQuitCheck && nrOfTries > maxTries)
				break; // this terrain piece is probably invalid! Early quit to increase
					   // performance.
			nrOfTries++;
			float2 pos;
			pos.x = RandomFloat(position.x, position.x + size.x);
			pos.y = RandomFloat(position.y, position.y + size.y);
			float height = terrain.getLocalHeightFromUV(pos);
			float3 lpos = float3(pos.x, height, pos.y);
			float3 wpos = float3::Transform(lpos, terrain.getMatrix());

			// check if valid position
			// above water
			if (wpos.y < 1)
				continue;
			// on flat ground
			float3 normal = terrain.getNormalFromPosition(wpos.x, wpos.z);
			if (normal.Dot(float3(0, 1, 0)) < 0.7f)
				continue;

			// insert straw
			straw.position = float3(pos.x, height, pos.y);
			straw.rotationY = RandomFloat(0, 2 * XM_PI);
			straw.height = RandomFloat(0, 1);
			m_straws.push_back(straw);
			earlyQuitCheck = false; // this is a valid terrain piece!
		}
		m_straws.shrink_to_fit();
	}
}

void GrassManager::GrassPatch::bind() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(Straw);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void GrassManager::GrassPatch::drawCall() { Renderer::draw(m_straws.size(), 0); }

bool GrassManager::GrassPatch::isEmpty() const { return m_straws.size() == 0; }

void GrassManager::update_strawBuffer() { m_cbuffer_settings.update(m_strawSetting); }

void GrassManager::update_animationBuffer(float time) { 
	m_animationSetting.time = time;
	m_cbuffer_animation.update(m_animationSetting); 
}

void GrassManager::init(Terrain& terrain, AreaTag tag) { 
	// set transformation
	setPosition(terrain.getPosition());
	setScale(terrain.getScale());
	setRotation(terrain.getRotation());

	// set grids (normalized)
	XMINT2 grids = terrain.getSplits(); 
	if (grids.x != 0 && grids.y != 0) {
		float strawsForArea = STRAW_PER_AREAUNIT * getScale().x * getScale().z;
		float strawsPerTile = strawsForArea / (grids.x * grids.y);

		size_t layers = (size_t)round(log2(max(grids.x, grids.y)));
		m_patches.initilize(float3(), float3(1.), layers);

		float2 tileSize(1.f / grids.x, 1.f / grids.y);
		for (size_t xx = 0; xx < grids.x; xx++) {
			for (size_t yy = 0; yy < grids.y; yy++) {
				// create grass patch
				float2 pos;
				pos.x = (float)xx / grids.x;
				pos.y = (float)yy / grids.y;
				GrassPatch patch;
				patch.generate(pos, tileSize, strawsPerTile, terrain);
				if (patch.isEmpty() == false) {
					m_patches.add(
						float3(pos.x, 0, pos.y), float3(tileSize.x, 1, tileSize.y), patch);
					GrassPatch* ptr_patch = &m_patches[m_patches.size() - 1];
					ptr_patch->createBuffer();
				}
			}
		}
	}

	// set settings
	setStrawAndAnimationSettings(tag);
}

void GrassManager::setStrawAndAnimationSettings(AreaTag tag) {
	switch (tag) {
	case Forest:
		m_visibility = true;
		m_strawSetting.baseWidth = 0.115f;
		m_strawSetting.heightRange = float2(0.568f, 1.f);
		m_strawSetting.noiseInterval = 30;
		m_strawSetting.color_top = float4(106 / 255.f, 138 / 255.f, 21 / 255.f, 1);
		m_strawSetting.color_bottom = float4(70 / 255.f, 93 / 255.f, 32 / 255.f, 1);

		m_animationSetting.speed = 30;
		m_animationSetting.noiseAnimInterval = 24.5f;
		m_animationSetting.offsetStrength = 0.26f;
		break;
	case Plains:
		m_visibility = true;
		m_strawSetting.baseWidth = 0.112f;
		m_strawSetting.heightRange = float2(0.359f, 0.5f);
		m_strawSetting.noiseInterval = 25;
		m_strawSetting.color_top = float4(106 / 255.f, 138 / 255.f, 21 / 255.f, 1);
		m_strawSetting.color_bottom = float4(70 / 255.f, 93 / 255.f, 32 / 255.f, 1);

		m_animationSetting.speed = 20;
		m_animationSetting.noiseAnimInterval = 7.4f;
		m_animationSetting.offsetStrength = 0.2f;
		break;
	case Desert:
		m_visibility = false;
		break;
	case Volcano:
		m_visibility = false;
		break;
	default:
		break;
	}
}

void GrassManager::quadtreeCull(vector<FrustumPlane> planes) {
	m_useCulling = true;
	// transform planes to local space
	float4x4 invWorldMatrix = getMatrix().Invert();
	float4x4 invWorldInvTraMatrix = getInversedTransposedMatrix().Invert();
	for (size_t i = 0; i < planes.size(); i++) {
		planes[i].m_position = float3::Transform(planes[i].m_position, invWorldMatrix);
		planes[i].m_normal = float3::TransformNormal(planes[i].m_normal, invWorldInvTraMatrix);
		planes[i].m_normal.Normalize();
	}
	// cull grids
	m_culledPatches = m_patches.cullElements(planes);
}

void GrassManager::imgui_animation() {
	if (ImGui::Begin("Animation Setting")) {
		ImGui::SliderFloat("speed", &m_animationSetting.speed, 0, 50);
		ImGui::SliderFloat("noiseInterval", &m_animationSetting.noiseAnimInterval, 0, 50);
		ImGui::SliderFloat("offsetStrength", &m_animationSetting.offsetStrength, 0, 1);
		ImGui::End();
	}
}

void GrassManager::clearCulling() {
	m_useCulling = false;
	m_culledPatches.clear();
}

void GrassManager::imgui_settings() {
	if (ImGui::Begin("Straw Setting")) {
		ImGui::SliderFloat("width", &m_strawSetting.baseWidth, 0, 1);
		ImGui::SliderFloat("min height", &m_strawSetting.heightRange.x, 0, 1);
		ImGui::SliderFloat("max height", &m_strawSetting.heightRange.y, 0, 1);
		ImGui::SliderFloat("noise interval", &m_strawSetting.noiseInterval, 0, 50);
		ImGui::ColorEdit3("color bottom", (float*)&m_strawSetting.color_bottom);
		ImGui::ColorEdit3("color top", (float*)&m_strawSetting.color_top);
		ImGui::End();
	}
}

void GrassManager::bindNoiseTexture(size_t slot) {
	Renderer::getDeviceContext()->GSSetShaderResources(slot, 1, m_tex_noise->view.GetAddressOf());
}

void GrassManager::draw() {
	imgui_settings();
	imgui_animation();

	if (m_visibility) {
		// straw setting cbuffer
		update_strawBuffer();
		m_cbuffer_settings.bindGS(SETTING_SLOT);
		// time cbuffer
		Scene* scene = SceneManager::getScene();
		update_animationBuffer(scene ? SceneManager::getScene()->m_timer.getTimePassed() : 0);
		m_cbuffer_animation.bindGS(CBUFFER_ANIMATION_SLOT);
		// noise size buffer
		m_cbuffer_noiseSize.bindGS(CBUFFER_NOISESIZE_SLOT);
		// shader
		m_shader.bindShadersAndLayout();
		// noise texture
		bindNoiseTexture(TEX_NOISE_SLOT);
		// world matrix
		GSBindMatrix(MATRIX_SLOT);

		Renderer::getInstance()->setRasterizer_noCulling(); // enable backface rendering
		if (m_useCulling) {
			for (size_t i = 0; i < m_culledPatches.size(); i++) {
				m_culledPatches[i]->bind();
				m_culledPatches[i]->drawCall();
			}
		}
		else {
			for (size_t i = 0; i < m_patches.size(); i++) {
				m_patches[i].bind();
				m_patches[i].drawCall();
			}
		}
		Renderer::getInstance()->setRasterizer_backfaceCulling(); // reset back to backface culling
	}
}

GrassManager::GrassManager() {
	m_tex_noise = TextureRepository::get("noise1.png");
	m_cbuffer_noiseSize.update(
		float4(m_tex_noise->description.Width, m_tex_noise->description.Height, 0, 0));
	if (!m_shader.isLoaded()) {
		D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
			{
				"Position",					 // "semantic" name in shader
				0,							 // "semantic" index (not used)
				DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{ "RotationY", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Height", 0, DXGI_FORMAT_R32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "ColorBrightness", 0, DXGI_FORMAT_R32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		m_shader.createShaders(L"VertexShader_grass.hlsl", L"GeometryShader_grass.hlsl", L"PixelShader_grass.hlsl", inputLayout, 4);
	}
}
