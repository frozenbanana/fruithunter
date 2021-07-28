#include "Terrain.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include <WICTextureLoader.h>
#include "Input.h"
#include "SceneManager.h"

#define MAX_HEIGHT_OFFSET 5.f

ShaderSet Terrain::m_shader_terrain;
ShaderSet Terrain::m_shader_terrain_brush;
ShaderSet Terrain::m_shader_terrain_onlyMesh;
ShaderSet Terrain::m_shader_grass;
ConstantBuffer<Brush> Terrain::m_buffer_brush;
ConstantBuffer<Terrain::StrawSetting> Terrain::m_cbuffer_settings;
ConstantBuffer<Terrain::AnimationSetting> Terrain::m_cbuffer_animation;
ConstantBuffer<Terrain::ColorBuffer> Terrain::m_cbuffer_color;
ConstantBuffer<float4> Terrain::m_cbuffer_noiseSize;

void Terrain::bindNoiseTexture(size_t slot) {
	Renderer::getDeviceContext()->GSSetShaderResources(
		slot, (UINT)1, m_tex_noise->getSRV().GetAddressOf());
}

void Terrain::update_strawBuffer() { m_cbuffer_settings.update(m_grass_strawSetting); }

void Terrain::update_animationBuffer(float time) {
	m_grass_animationSetting.time = time;
	m_cbuffer_animation.update(m_grass_animationSetting);
}

void Terrain::setStrawAndAnimationSettings(AreaTag tag) {
	switch (tag) {
	case Forest:
		m_grass_visibility = true;
		m_grass_strawSetting.baseWidth = 0.115f;
		m_grass_strawSetting.heightRange = float2(0.568f, 1.f);
		m_grass_strawSetting.noiseInterval = 6.2f;
		m_grass_strawSetting.color_top = float4(106 / 255.f, 138 / 255.f, 21 / 255.f, 1);
		m_grass_strawSetting.color_bottom = float4(70 / 255.f, 93 / 255.f, 32 / 255.f, 1);

		m_grass_animationSetting.speed = 13.7f;
		m_grass_animationSetting.noiseAnimInterval = 5.f;
		m_grass_animationSetting.offsetStrength = 0.26f;
		break;
	case Plains:
		m_grass_visibility = true;
		m_grass_strawSetting.baseWidth = 0.112f;
		m_grass_strawSetting.heightRange = float2(0.359f, 0.5f);
		m_grass_strawSetting.noiseInterval = 6.2f;
		m_grass_strawSetting.color_top = float4(106 / 255.f, 138 / 255.f, 21 / 255.f, 1);
		m_grass_strawSetting.color_bottom = float4(70 / 255.f, 93 / 255.f, 32 / 255.f, 1);

		m_grass_animationSetting.speed = 12;
		m_grass_animationSetting.noiseAnimInterval = 3.25f;
		m_grass_animationSetting.offsetStrength = 0.2f;
		break;
	case Desert:
		m_grass_visibility = false;
		break;
	case Volcano:
		m_grass_visibility = false;
		break;
	default:
		break;
	}
}

void Terrain::setColorSettings(AreaTag tag) {
	switch (tag) {
	case Forest:
		m_colorBuffer.color_flat = float4(61 / 255.f, 105 / 255.f, 22 / 255.f, 1);
		m_colorBuffer.color_tilt = float4(74 / 255.f, 65 / 255.f, 25 / 255.f, 1);
		m_colorBuffer.intensityRange = float2(0.9f, 1.1f);
		break;
	case Plains:
		m_colorBuffer.color_flat = float4(74 / 255.f, 106 / 255.f, 31 / 255.f, 1);
		m_colorBuffer.color_tilt = float4(104 / 255.f, 80 / 255.f, 29 / 255.f, 1);
		m_colorBuffer.intensityRange = float2(0.9f, 1.1f);
		break;
	case Desert:
		m_colorBuffer.color_flat = float4(208 / 255.f, 208 / 255.f, 175 / 255.f, 1);
		m_colorBuffer.color_tilt = float4(110 / 255.f, 110 / 255.f, 88 / 255.f, 1);
		m_colorBuffer.intensityRange = float2(0.9f, 1.1f);
		break;
	case Volcano:
		m_colorBuffer.color_flat = float4(51 / 255.f, 54 / 255.f, 83 / 255.f, 1);
		m_colorBuffer.color_tilt = float4(30 / 255.f, 37 / 255.f, 66 / 255.f, 1);
		m_colorBuffer.intensityRange = float2(0.9f, 1.1f);
		break;
	default:
		break;
	}
}

void Terrain::imgui_settings() {
	if (ImGui::Begin("Straw Setting")) {
		ImGui::SliderFloat("width", &m_grass_strawSetting.baseWidth, 0, 1);
		ImGui::SliderFloat("min height", &m_grass_strawSetting.heightRange.x, 0, 1);
		ImGui::SliderFloat("max height", &m_grass_strawSetting.heightRange.y, 0, 1);
		ImGui::SliderFloat("noise interval", &m_grass_strawSetting.noiseInterval, 0, 50);
		ImGui::ColorEdit3("color bottom", (float*)&m_grass_strawSetting.color_bottom);
		ImGui::ColorEdit3("color top", (float*)&m_grass_strawSetting.color_top);
	}
	ImGui::End();
}

void Terrain::imgui_animation() {
	if (ImGui::Begin("Animation Setting")) {
		ImGui::SliderFloat("speed", &m_grass_animationSetting.speed, 0, 50);
		ImGui::SliderFloat("noiseInterval", &m_grass_animationSetting.noiseAnimInterval, 0, 50);
		ImGui::SliderFloat("offsetStrength", &m_grass_animationSetting.offsetStrength, 0, 1);
	}
	ImGui::End();
}

void Terrain::fillSubMeshes() {
	if (m_gridSize.x != 0 && m_gridSize.y != 0 && m_tileSize.x != 0 && m_tileSize.y != 0) {
		// initilize quadtree
		size_t layers = (size_t)round(log2(max(m_gridSize.x, m_gridSize.y)));
		m_quadtree.initilize(float3(0, 0, 0), float3(1.f, 1.f, 1.f), layers);
		m_quadtree.reserve((size_t)m_gridSize.x * (size_t)m_gridSize.y);
		// initilize subMeshes
		float2 cellSize = float2(1.f / m_gridSize.x, 1.f / m_gridSize.y);
		m_subMeshes.resize(m_gridSize.x);
		for (int ixx = 0; ixx < m_gridSize.x; ixx++) {
			m_subMeshes[ixx].resize(m_gridSize.y);
			for (int iyy = 0; iyy < m_gridSize.y; iyy++) {
				float2 position = cellSize * float2((float)ixx, (float)iyy);

				// Terrain
				m_subMeshes[ixx][iyy].generate_terrain(
					m_tileSize, XMINT2(ixx, iyy), m_heightmapMesh);

				// Grass
				float strawsForArea = STRAW_PER_AREAUNIT * getScale().x * getScale().z;
				float strawsPerTile = strawsForArea / (m_gridSize.x * m_gridSize.y);
				m_subMeshes[ixx][iyy].generate_grass(
					position, cellSize, (size_t)strawsPerTile, m_heightmapMesh, getMatrix());

				// add 2D index to quadtree
				m_quadtree.add(float3(position.x, 0, position.y),
					float3(cellSize.x, 1.f, cellSize.y), XMINT2(ixx, iyy));
			}
		}
	}
	else {
		// invalid size
	}
}

void Terrain::loadFromFile_binary(ifstream& file) {
	// transformation
	((Transformation*)this)->stream_read(file);
	// tile size
	file.read((char*)&m_tileSize, sizeof(XMINT2));
	// grid size
	file.read((char*)&m_gridSize, sizeof(XMINT2));
	// heightmap points
	m_heightmapMesh.loadFromFile_binary(file);
	// generate mesh
	fillSubMeshes();
}

void Terrain::storeToFile_binary(ofstream& file) {
	// transformation
	((Transformation*)this)->stream_write(file);
	// tile size
	file.write((char*)&m_tileSize, sizeof(XMINT2));
	// grid size
	file.write((char*)&m_gridSize, sizeof(XMINT2));
	// heightmapMesh
	m_heightmapMesh.storeToFile_binary(file);
}

bool Terrain::boxInsideFrustum(float3 boxPos, float3 boxSize, const vector<FrustumPlane>& planes) {
	float4x4 mWorld = getMatrix();
	// normalized box points
	float3 boxPoints[8] = { float3(0, 0, 0), float3(1, 0, 0), float3(1, 0, 1), float3(0, 0, 1),
		float3(0, 1, 0), float3(1, 1, 0), float3(1, 1, 1), float3(0, 1, 1) };
	// transform points to world space
	for (size_t i = 0; i < 8; i++) {
		boxPoints[i] =
			float3::Transform(boxPos + float3(boxPoints[i].x * boxSize.x,
										   boxPoints[i].y * boxSize.y, boxPoints[i].z * boxSize.z),
				mWorld);
	}
	// for each plane
	for (size_t plane_i = 0; plane_i < planes.size(); plane_i++) {
		// find diagonal points
		float3 boxDiagonalPoint1, boxDiagonalPoint2;
		float largestDot = -1;
		for (size_t j = 0; j < 4; j++) {
			float3 p1 = boxPoints[j];
			float3 p2 = boxPoints[4 + (j + 2) % 4];
			float3 pn = p1 - p2;
			pn.Normalize();
			float dot = abs(pn.Dot(planes[plane_i].m_normal));
			if (dot > largestDot) {
				largestDot = dot;
				boxDiagonalPoint1 = p1;
				boxDiagonalPoint2 = p2;
			}
		}
		// compare points
		float min = (boxDiagonalPoint1 - planes[plane_i].m_position).Dot(planes[plane_i].m_normal);
		float max = (boxDiagonalPoint2 - planes[plane_i].m_position).Dot(planes[plane_i].m_normal);
		if (min > max) {
			// switch
			float temp = max;
			max = min;
			min = temp;
		}
		if (min > 0) {
			// outside
			return false;
		}
	}
	return true;
}

XMINT2 Terrain::getGridPointSize() const {
	return XMINT2(m_gridSize.x * m_tileSize.x + 1, m_gridSize.y * m_tileSize.y + 1);
}

void Terrain::hq_apply(shared_ptr<HQTerrainElement> sample) {
	m_gridSize = sample->subSize;
	m_tileSize = sample->divisions;
	m_heightmapMesh.applyAbstractData(sample->data);
	// rebuild mesh
	fillSubMeshes();
}

shared_ptr<HQTerrainElement> Terrain::hq_fetch() const {
	shared_ptr<HQTerrainElement> sample = make_shared<HQTerrainElement>();
	sample->subSize = getSubSize();
	sample->divisions = getSplits();
	m_heightmapMesh.getAbstractData(sample->data);
	return sample;
}

XMINT2 Terrain::getSplits() const { return m_gridSize; }

XMINT2 Terrain::getSubSize() const { return m_tileSize; }

size_t Terrain::getTriangleCount() const {
	size_t sum = 0;
	for (size_t x = 0; x < m_subMeshes.size(); x++) {
		for (size_t y = 0; y < m_subMeshes[x].size(); y++) {
			sum += m_subMeshes[x][y].getTriangleCount();
		}
	}
	return sum;
}

void Terrain::initilize(string filename, XMINT2 subsize, XMINT2 splits) {
	// grass noise texture
	m_tex_noise = TextureRepository::get("noise_grass.png");
	// grass noise buffer
	m_cbuffer_noiseSize.update(
		float4((float)m_tex_noise->getSize().x, (float)m_tex_noise->getSize().y, 0.f, 0.f));

	// load terrain
	build(filename, subsize, splits);
}

void Terrain::build(string heightmapName, XMINT2 subSize, XMINT2 splits) {
	m_tileSize = subSize;
	m_gridSize = splits;
	m_heightmapMesh.init(heightmapName, getGridPointSize());
	fillSubMeshes();
}

void Terrain::changeSize(XMINT2 tileSize, XMINT2 gridSize) {
	m_tileSize = tileSize;
	m_gridSize = gridSize;

	m_heightmapMesh.changeSize(getGridPointSize());

	fillSubMeshes();
}

void Terrain::editMesh(const Brush& brush, Brush::Type type) {
	float4x4 matWorld = getMatrix();
	float dt = SceneManager::getScene()->getDeltaTime();

	// apply brush to mesh
	m_heightmapMesh.editMesh(brush, type, dt, matWorld);

	// update affected cells
	float2 b_position = float2(brush.position.x, brush.position.z);
	float2 b_wP1 = b_position - float2(brush.radius, brush.radius);
	float2 b_wP2 = b_position + float2(brush.radius, brush.radius);
	float2 cellSize = float2(1.f / m_gridSize.x, 1.f / m_gridSize.y);
	for (size_t gix = 0; gix < m_gridSize.x; gix++) {
		for (size_t giy = 0; giy < m_gridSize.y; giy++) {
			XMINT2 gridIndex((int)gix, (int)giy);
			float2 gPosition = cellSize * float2((float)gridIndex.x, (float)gridIndex.y);
			float3 wP1_f3 = float3::Transform(float3(gPosition.x, 0, gPosition.y), matWorld);
			float3 wP2_f3 = float3::Transform(
				float3(gPosition.x + cellSize.x, 0, gPosition.y + cellSize.y), matWorld);
			float2 wP1 = float2(wP1_f3.x, wP1_f3.z);
			float2 wP2 = float2(wP2_f3.x, wP2_f3.z);

			if (wP2.x > b_wP1.x && wP2.y > b_wP1.y && wP1.x < b_wP2.x && wP1.y < b_wP2.y) {
				// grid cell inside
				m_subMeshes[gix][giy].generate_terrain(
					m_tileSize, XMINT2((int)gix, (int)giy), m_heightmapMesh);

				float strawsForArea = STRAW_PER_AREAUNIT * getScale().x * getScale().z;
				float strawsPerTile = strawsForArea / (m_gridSize.x * m_gridSize.y);
				m_subMeshes[gridIndex.x][gridIndex.y].generate_grass(
					gPosition, cellSize, (size_t)strawsPerTile, m_heightmapMesh, getMatrix());
			}
		}
	}
}

void Terrain::smoothMesh(float distance) {
	m_heightmapMesh.smoothMesh(distance, getScale());
	fillSubMeshes();
}

bool Terrain::pointInsideTerrainBoundingBox(float3 point) {
	float3 pos = point;
	float4x4 mTerrainInvWorld = getMatrix().Invert();
	pos = float3::Transform(pos, mTerrainInvWorld);
	if (pos.x >= 0 && pos.x < 1. && pos.y >= 0 && pos.y < 1. && pos.z >= 0 && pos.z < 1.)
		return true;
	else
		return false;
}

float Terrain::getHeightFromPosition(float x, float z) {
	// map to local
	float3 position(x, 0, z);
	float4x4 mTerrainWorld = getMatrix();
	float4x4 mTerrainInvWorld = mTerrainWorld.Invert();
	position = float3::Transform(position, mTerrainInvWorld);
	// get local height
	float height = m_heightmapMesh.getHeightFromUV(float2(position.x, position.z));
	// map to world
	float3 pos(0, height, 0);
	return float3::Transform(pos, mTerrainWorld).y;
}

float3 Terrain::getNormalFromPosition(float x, float z) {
	// map to local
	float3 position(x, 0, z);
	float4x4 mTerrainWorld = getMatrix();
	float4x4 mTerrainInvWorld = mTerrainWorld.Invert();
	position = float3::Transform(position, mTerrainInvWorld);
	// get local normal
	float3 normal = m_heightmapMesh.getNormalFromUV(float2(position.x, position.z));
	// map to world
	float3 cn = float3::Transform(normal, mTerrainWorld.Invert().Transpose());
	cn.Normalize();

	return cn;
}

/*
 * Returns scale of direction until collision, point+direction*scale = collisionPoint. '-1' means
 * miss!
 */
float Terrain::castRay(float3 point, float3 direction) {
	if (direction.Length() > 0) {
		// convert to local space
		float4x4 mTerrainWorld = getMatrix();
		float4x4 mTerrainInvWorld = mTerrainWorld.Invert();
		float3 startPoint = float3::Transform(point, mTerrainInvWorld);
		float3 endPoint = float3::Transform(point + direction, mTerrainInvWorld);

		// cast local ray
		float t = m_heightmapMesh.castRay(startPoint, endPoint);
		if (t != -1) {
			// return intersection to world coord
			float3 intersectPoint = startPoint + (endPoint - startPoint) * t;
			intersectPoint = float3::Transform(intersectPoint, mTerrainWorld);

			// return length of ray until intersection
			float l = (intersectPoint - point).Length() / direction.Length();
			if (l > 0 && l <= 1)
				return l;
			else
				return -1;
		}
		else
			return -1;
	}
	return -1;
}

void Terrain::clearCulling() {
	m_useCulling = false;
	m_culledGrids.clear();
}

void Terrain::quadtreeCull(vector<FrustumPlane> planes) {
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
	vector<XMINT2*> copies = m_quadtree.cullElements(planes);
	m_culledGrids.resize(copies.size());
	for (size_t i = 0; i < copies.size(); i++)
		m_culledGrids[i] = *copies[i];
}

void Terrain::boundingBoxCull(CubeBoundingBox bb) {
	m_useCulling = true;
	// transform planes to local space
	float4x4 invWorldMatrix = getMatrix().Invert();
	float4x4 invWorldInvTraMatrix = getInversedTransposedMatrix().Invert();

	bb.m_position = float3::Transform(bb.m_position, invWorldMatrix);
	bb.m_size = bb.m_size / getScale();
	// cull grids
	vector<XMINT2*> copies = m_quadtree.cullElements(bb);
	m_culledGrids.resize(copies.size());
	for (size_t i = 0; i < copies.size(); i++)
		m_culledGrids[i] = *copies[i];
}

void Terrain::draw() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	// bind shaders
	m_shader_terrain.bindShadersAndLayout();
	// color buffer
	update_colorBuffer();
	m_cbuffer_color.bindPS(CBUFFER_COLOR);
	// bind world matrix
	VSBindMatrix(MATRIX_SLOT);
	// draw
	if (m_useCulling) {
		for (size_t i = 0; i < m_culledGrids.size(); i++) {
			SubGrid* sub = &m_subMeshes[m_culledGrids[i].x][m_culledGrids[i].y];
			sub->bind_terrain();
			sub->drawCall_terrain();
		}
	}
	else {
		// draw grids
		for (int xx = 0; xx < m_gridSize.x; xx++) {
			for (int yy = 0; yy < m_gridSize.y; yy++) {
				SubGrid* sub = &m_subMeshes[xx][yy];
				sub->bind_terrain();
				sub->drawCall_terrain();
			}
		}
	}
}

void Terrain::draw_onlyMesh() {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	// bind shaders
	m_shader_terrain_onlyMesh.bindShadersAndLayout();
	// bind world matrix
	VSBindMatrix(MATRIX_SLOT);
	// draw
	if (m_useCulling) {
		for (size_t i = 0; i < m_culledGrids.size(); i++) {
			SubGrid* sub = &m_subMeshes[m_culledGrids[i].x][m_culledGrids[i].y];
			sub->bind_terrain();
			sub->drawCall_terrain();
		}
	}
	else {
		// draw grids
		for (int xx = 0; xx < m_gridSize.x; xx++) {
			for (int yy = 0; yy < m_gridSize.y; yy++) {
				SubGrid* sub = &m_subMeshes[xx][yy];
				sub->bind_terrain();
				sub->drawCall_terrain();
			}
		}
	}
}

void Terrain::draw_brush(const Brush& brush) {
	ID3D11DeviceContext* deviceContext = Renderer::getDeviceContext();
	// bind shaders
	m_shader_terrain_brush.bindShadersAndLayout();
	// color buffer
	update_colorBuffer();
	m_cbuffer_color.bindPS(CBUFFER_COLOR);
	// bind world matrix
	VSBindMatrix(MATRIX_SLOT);
	// bind brush buffer
	m_buffer_brush.update(brush);
	m_buffer_brush.bindPS(7);
	// draw
	if (m_useCulling) {
		for (size_t i = 0; i < m_culledGrids.size(); i++) {
			SubGrid* sub = &m_subMeshes[m_culledGrids[i].x][m_culledGrids[i].y];
			sub->bind_terrain();
			sub->drawCall_terrain();
		}
	}
	else {
		// draw grids
		for (int xx = 0; xx < m_gridSize.x; xx++) {
			for (int yy = 0; yy < m_gridSize.y; yy++) {
				SubGrid* sub = &m_subMeshes[xx][yy];
				sub->bind_terrain();
				sub->drawCall_terrain();
			}
		}
	}
}

void Terrain::draw_grass() {
	if (m_grass_visibility) {
		// straw setting cbuffer
		update_strawBuffer();
		m_cbuffer_settings.bindGS(SETTING_SLOT);
		// time cbuffer
		Scene* scene = SceneManager::getScene();
		update_animationBuffer(
			scene ? (float)SceneManager::getScene()->getTimePassedAsSeconds() : 0);
		m_cbuffer_animation.bindGS(CBUFFER_ANIMATION_SLOT);
		// noise size buffer
		m_cbuffer_noiseSize.bindGS(CBUFFER_NOISESIZE_SLOT);
		// shader
		m_shader_grass.bindShadersAndLayout();
		// noise texture
		bindNoiseTexture(TEX_NOISE_SLOT);
		// world matrix
		GSBindMatrix(MATRIX_SLOT);

		Renderer::getInstance()->setRasterizer_CullNone(); // enable backface rendering
		if (m_useCulling) {
			for (size_t i = 0; i < m_culledGrids.size(); i++) {
				SubGrid* sub = &m_subMeshes[m_culledGrids[i].x][m_culledGrids[i].y];
				sub->bind_grass();
				sub->drawCall_grass();
			}
		}
		else {
			for (int xx = 0; xx < m_gridSize.x; xx++) {
				for (int yy = 0; yy < m_gridSize.y; yy++) {
					SubGrid* sub = &m_subMeshes[xx][yy];
					sub->bind_grass();
					sub->drawCall_grass();
				}
			}
		}
		Renderer::getInstance()
			->setRasterizer_CullCounterClockwise(); // reset back to backface culling
	}
}
//
//Terrain::Terrain(const Terrain& other) : Transformation(other), Fragment(other) { *this = other; }

Terrain::Terrain(string filename, XMINT2 subsize, XMINT2 splits)
	: Fragment(Fragment::Type::terrain), HistoryQueue<HQTerrainElement>(15) {
	initilize(filename, subsize, splits);
	if (!m_shader_terrain.isLoaded()) {
		D3D11_INPUT_ELEMENT_DESC inputLayout_onlyMesh[] = {
			{
				"Position",					 // "semantic" name in shader
				0,							 // "semantic" index (not used)
				DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{ "TexCoordinate", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "ColorIntensity", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		m_shader_terrain.createShaders(L"VertexShader_terrain.hlsl", NULL,
			L"PixelShader_terrain.hlsl", inputLayout_onlyMesh, 4);
		m_shader_terrain_brush.createShaders(L"VertexShader_terrain.hlsl", NULL,
			L"PixelShader_terrain_brush.hlsl", inputLayout_onlyMesh, 4);
		m_shader_terrain_onlyMesh.createShaders(L"VertexShader_terrain.hlsl", NULL,
			L"PixelShader_terrain_onlyMesh.hlsl", inputLayout_onlyMesh, 4);
	}

	if (!m_shader_grass.isLoaded()) {
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
		m_shader_grass.createShaders(L"VertexShader_grass.hlsl", L"GeometryShader_grass.hlsl",
			L"PixelShader_grass.hlsl", inputLayout, 4);
	}
}

Terrain::~Terrain() {}

//Terrain& Terrain::operator=(const Terrain& other) {
//	m_tileSize = other.m_tileSize;
//	m_gridSize = other.m_gridSize;
//	m_subMeshes = other.m_subMeshes;
//	m_heightmapMesh = other.m_heightmapMesh;
//	m_quadtree = other.m_quadtree;
//	m_culledGrids = other.m_culledGrids;
//	m_useCulling = other.m_useCulling;
//
//	m_grass_visibility = other.m_grass_visibility;
//	m_grass_strawSetting = other.m_grass_strawSetting;
//	m_grass_animationSetting = other.m_grass_animationSetting;
//	m_tex_noise = other.m_tex_noise;
//
//	return *this;
//}

size_t Terrain::SubGrid::getTriangleCount() const { return m_vertices.size() / 3; }

void Terrain::SubGrid::generate_terrain(XMINT2 tileSize, XMINT2 gridIndex, HeightmapMesh& hmMesh) {
	const XMINT2 order[6] = { // tri1
		XMINT2(1, 1), XMINT2(0, 0), XMINT2(0, 1),
		// tri2
		XMINT2(0, 0), XMINT2(1, 1), XMINT2(1, 0)
	};

	vector<TerrainVertex>* vertices = &m_vertices;
	vertices->clear();
	vertices->reserve((size_t)tileSize.x * tileSize.y * 6 + 6);

	XMINT2 indexStart(gridIndex.x * tileSize.x, gridIndex.y * tileSize.y);
	XMINT2 indexStop(indexStart.x + tileSize.x, indexStart.y + tileSize.y);
	// ground platform
	for (size_t i = 0; i < 6; i++) {
		Vertex groundVertex = hmMesh[order[i].x ? indexStop.x : indexStart.x]
									[order[i].y ? indexStop.y : indexStart.y];
		groundVertex.position.y = 0;
		vertices->push_back(TerrainVertex(groundVertex));
	}
	// terrain triangles
	Vertex v[6];
	for (int xx = indexStart.x; xx < indexStop.x; xx++) {
		for (int yy = indexStart.y; yy < indexStop.y; yy++) {
			for (int i = 0; i < 6; i++) {
				XMINT2 index(xx + order[i].x, yy + order[i].y);
				v[i] = hmMesh[index.x][index.y];
			}
			for (size_t i = 0; i < 2; i++) {
				size_t ii = i * 3;
				if (!(v[ii + 0].position.y == 0 && v[ii + 1].position.y == 0 &&
						v[ii + 2].position.y == 0)) {
					float intensity = RandomFloat();
					vertices->push_back(TerrainVertex(v[ii + 0], intensity));
					vertices->push_back(TerrainVertex(v[ii + 1], intensity));
					vertices->push_back(TerrainVertex(v[ii + 2], intensity));
				}
			}
		}
	}

	// flatshade
	if (FLAT_SHADING) {
		// fix normals to flat shading
		for (size_t i = 0; i < vertices->size(); i += 3) {
			float3 p1 = (*vertices)[i + 0].position;
			float3 p2 = (*vertices)[i + 1].position;
			float3 p3 = (*vertices)[i + 2].position;
			float3 normal = (p2 - p1).Cross(p3 - p1);
			normal.Normalize();
			(*vertices)[i + 0].normal = normal;
			(*vertices)[i + 1].normal = normal;
			(*vertices)[i + 2].normal = normal;
		}
	}
	// EDGE SHADING
	if (EDGE_SHADING) {
		for (size_t i = 0; i < vertices->size(); i += 3) {
			float3 p1 = (*vertices)[i + 0].position;
			float3 p2 = (*vertices)[i + 1].position;
			float3 p3 = (*vertices)[i + 2].position;
			float3 normal = (p2 - p1).Cross(p3 - p1); // flat normal
			normal.Normalize();
			float3 pn1 = (*vertices)[i + 0].normal;
			float3 pn2 = (*vertices)[i + 1].normal;
			float3 pn3 = (*vertices)[i + 2].normal;

			(*vertices)[i + 0].normal = (pn1.Dot(normal) > EDGE_THRESHOLD ? pn1 : normal);
			(*vertices)[i + 1].normal = (pn2.Dot(normal) > EDGE_THRESHOLD ? pn2 : normal);
			(*vertices)[i + 2].normal = (pn3.Dot(normal) > EDGE_THRESHOLD ? pn3 : normal);
		}
	}

	createBuffer_terrain();
}

void Terrain::SubGrid::createBuffer_grass() {
	if (m_straws.size() > 0) {
		// vertex buffer
		m_vbuffer_grass.Reset();
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = (UINT)m_straws.size() * sizeof(Straw);
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_straws.data();
		HRESULT res =
			Renderer::getDevice()->CreateBuffer(&bufferDesc, &data, m_vbuffer_grass.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(
				"[Terrain::SubGrid.createBuffers] Failed creating vertex buffer(Straw)!\n", res);
	}
}

void Terrain::SubGrid::createBuffer_terrain() {
	if (m_vertices.size() > 0) {
		m_vbuffer_terrain.Reset();
		D3D11_BUFFER_DESC bufferDesc;
		memset(&bufferDesc, 0, sizeof(bufferDesc));
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = (UINT)m_vertices.size() * sizeof(TerrainVertex);
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = m_vertices.data();
		HRESULT res = Renderer::getDevice()->CreateBuffer(
			&bufferDesc, &data, m_vbuffer_terrain.GetAddressOf());
		if (FAILED(res))
			ErrorLogger::logError(
				"[Terrain::SubGrid.createBuffers] Failed creating vertex buffer(Terrain)!\n", res);
	}
}

void Terrain::SubGrid::bind_terrain() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(TerrainVertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vbuffer_terrain.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Terrain::SubGrid::drawCall_terrain() { Renderer::draw(m_vertices.size(), 0); }

void Terrain::SubGrid::bind_grass() {
	auto deviceContext = Renderer::getDeviceContext();
	UINT strides = sizeof(Straw);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, m_vbuffer_grass.GetAddressOf(), &strides, &offset);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
}

void Terrain::SubGrid::drawCall_grass() { Renderer::draw(m_straws.size(), 0); }

void Terrain::SubGrid::generate_grass(
	float2 position, float2 size, size_t count, HeightmapMesh& hmMesh, float4x4 worldMatrix) {

	m_straws.clear();
	if (count > 0 && hmMesh.containsValidPosition(position, size, worldMatrix)) {
		float4x4 worldInvTraMatrix = worldMatrix.Invert().Transpose();
		m_straws.reserve(count);
		for (size_t i = 0; i < count; i++) {
			float2 pos;
			pos.x = RandomFloat(position.x, position.x + size.x);
			pos.y = RandomFloat(position.y, position.y + size.y);
			float height = hmMesh.getHeightFromUV(pos);
			float3 lpos = float3(pos.x, height, pos.y);
			float3 wpos = float3::Transform(lpos, worldMatrix);

			// check if valid position
			// above water
			if (wpos.y < 1)
				continue; // invalid
			// on flat ground
			float3 normal = hmMesh.getNormalFromUV(float2(pos));
			normal = Normalize(float3::Transform(normal, worldInvTraMatrix));
			if (normal.Dot(float3(0, 1, 0)) < 0.7f)
				continue; // invalid

			// insert straw
			Straw straw;
			straw.position = float3(pos.x, height, pos.y);
			straw.rotationY = RandomFloat(0, 2 * XM_PI);
			straw.height = RandomFloat(0, 1);
			m_straws.push_back(straw);
		}
		m_straws.shrink_to_fit();
	}
	createBuffer_grass();
}

void Terrain::SubGrid::operator=(const SubGrid& other) {
	m_vertices = other.m_vertices;
	m_straws = other.m_straws;
	createBuffer_terrain();
	createBuffer_grass();
}

void Terrain::imgui_color() {
	if (ImGui::Begin("color buffer")) {
		ImGui::ColorEdit3("flat color", (float*)&m_colorBuffer.color_flat);
		ImGui::ColorEdit3("tilt color", (float*)&m_colorBuffer.color_tilt);
		ImGui::SliderFloat("min intensity", &m_colorBuffer.intensityRange.x, 0, 2);
		ImGui::SliderFloat("max intensity", &m_colorBuffer.intensityRange.y, 0, 2);
	}
	ImGui::End();
}

void Terrain::update_colorBuffer() { m_cbuffer_color.update(m_colorBuffer); }

bool Terrain::validPosition(float3 pos) {
	float4x4 wMat = getMatrix();
	float3 lPos = float3::Transform(pos, wMat.Invert());
	return m_heightmapMesh.validPosition(float2(lPos.x, lPos.z), wMat);
}