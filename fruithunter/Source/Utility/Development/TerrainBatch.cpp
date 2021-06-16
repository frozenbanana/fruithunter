#include "TerrainBatch.h"

void TerrainBatch::remove(size_t index) {
	if (index >= 0 && index < size()) {
		erase(begin() + index);
	}
}

void TerrainBatch::add(float3 position, float3 scale, string heightmapFilename, XMINT2 subSize,
	XMINT2 division, float3 wind, AreaTag tag) {

	push_back(make_shared<Environment>()); // create new
	back()->initilize(heightmapFilename, subSize, division);
	back()->setPosition(position);
	back()->setScale(scale);
	back()->setWind(wind);
	back()->setTag(tag);
}

void TerrainBatch::add(shared_ptr<Environment> environment) { push_back(environment); }

Environment* TerrainBatch::getTerrainFromPosition(float3 position) {
	for (size_t i = 0; i < size(); i++) {
		if (at(i)->pointInsideTerrainBoundingBox(float3(position.x, 0.0, position.z))) {
			return at(i).get();
		}
	}
	return nullptr;
}

int TerrainBatch::getTerrainIndexFromPosition(float3 position) const {
	for (size_t i = 0; i < size(); i++) {
		if (at(i)->pointInsideTerrainBoundingBox(float3(position.x, 0.0, position.z))) {
			return (int)i;
		}
	}
	return -1;
}

float TerrainBatch::getHeightFromPosition(float3 position) const {
	float highest = 0;
	for (size_t i = 0; i < size(); i++) {
		float h = at(i)->getHeightFromPosition(position.x, position.z);
		if (h > highest)
			highest = h;
	}
	return highest;
}

float3 TerrainBatch::getNormalFromPosition(float3 position) const {
	float highest = 0;
	float3 normal = float3(0, 1, 0);
	for (size_t i = 0; i < size(); i++) {
		float h = at(i)->getHeightFromPosition(position.x, position.z);
		if (h > highest) {
			highest = h;
			normal = at(i)->getNormalFromPosition(position.x, position.z);
		}
	}
	return normal;
}

float TerrainBatch::castRay(float3 point, float3 direction) {
	float intersection = -1;
	for (size_t i = 0; i < size(); i++) {
		float l = at(i)->castRay(point, direction);
		if (l != -1 && (intersection == -1 || l < intersection)) {
			intersection = l;
		}
	}
	return intersection;
}

bool TerrainBatch::validPosition(float3 pos) {
	// find heighest terrain
	int ter_i = -1;
	float ter_height = 0;
	for (size_t i = 0; i < size(); i++) {
		float height = at(i)->getHeightFromPosition(pos.x, pos.z);
		if (ter_i == -1 || (height > ter_height)) {
			ter_i = i;
			ter_height = height;
		}
	}
	if (ter_i != -1) {
		// found terrain
		return at(ter_i)->validPosition(pos);
	}
	return false; // didnt find any terrain, return invalid position
}

void TerrainBatch::clearCulling() {
	for (size_t i = 0; i < size(); i++)
		at(i)->clearCulling();
}

void TerrainBatch::quadtreeCull(const vector<FrustumPlane>& planes) {
	for (size_t i = 0; i < size(); i++)
		at(i)->quadtreeCull(planes);
}

void TerrainBatch::draw() {
	for (size_t i = 0; i < size(); i++)
		at(i)->draw();
}

void TerrainBatch::draw_brush(const Brush& brush) {
	for (size_t i = 0; i < size(); i++)
		at(i)->draw_brush(brush);
}

void TerrainBatch::draw_onlyMesh() {
	for (size_t i = 0; i < size(); i++)
		at(i)->draw_onlyMesh();
}

void TerrainBatch::draw_grass() {
	for (size_t i = 0; i < size(); i++)
		at(i)->draw_grass();
}

void TerrainBatch::editMesh(const Brush& brush, Brush::Type type) {
	for (size_t i = 0; i < size(); i++)
		at(i)->editMesh(brush, type);
}

void TerrainBatch::editMesh_pop() {
	for (size_t i = 0; i < size(); i++)
		at(i)->editMesh_pop();
}

void TerrainBatch::editMesh_push() {
	for (size_t i = 0; i < size(); i++)
		at(i)->editMesh_push();
}

Environment::Environment(string filename, XMINT2 subsize, XMINT2 splits, float3 wind, AreaTag tag)
	: Terrain(filename, subsize, splits) {
	setWind(wind);
	setTag(tag);
}

void Environment::setWind(float3 wind) { m_wind = wind; }

void Environment::setTag(AreaTag tag) {
	m_tag = tag;
	setStrawAndAnimationSettings(tag);
	setColorSettings(tag);
}

void Environment::setFruitSpawns(int fruitSpawns[NR_OF_FRUITS]) {
	memcpy(m_fruitSpawn, fruitSpawns, sizeof(int) * NR_OF_FRUITS);
}

float3 Environment::getWindStatic() const { return m_wind; }

AreaTag Environment::getTag() const { return m_tag; }

int Environment::getFruitCount(FruitType type) const { return m_fruitSpawn[type]; }

float3 Environment::getRandomSpawnPoint() {
	float3 point;
	size_t iterator = 0;
	size_t tries = 100;
	const float adjacentCheckDistance = 1;
	bool valid;
	do {
		valid = true;
		point = float3::Transform(float3(RandomFloat(0, 1), 0, RandomFloat(0, 1)), getMatrix());
		if (!validPosition(point))
			valid = false;
		for (size_t i = 0; i < 4; i++) {
			float r = ((float)i / 4) * XM_PI * 2;
			float3 offset = float3(cos(r), 0, sin(r)) * adjacentCheckDistance;
			if (!validPosition(point + offset))
				valid = false;
		}
		iterator++;
	} while (iterator < tries && !valid);
	if (iterator == tries)
		ErrorLogger::logError(
			"(Environment::getRandomSpawnPoint) Failed finding a spawn point for fruit!");
	return point;
}

void Environment::loadFromBinFile(string path) {
	fstream file;
	file.open(path, ios::in | ios::binary);
	if (file.is_open()) {
		// area
		AreaTag tag;
		file.read((char*)&tag, sizeof(AreaTag));
		// wind
		file.read((char*)&m_wind, sizeof(float3));
		// fruit spawns
		file.read((char*)m_fruitSpawn, sizeof(int) * NR_OF_FRUITS);
		// terrain
		loadFromFile_binary(file);

		setTag(tag);

		file.close();
	}
	else
		ErrorLogger::logError(
			"(Environment) Failed loading environment from file! path: " + path, HRESULT());
}

void Environment::storeToBinFile(string path) {
	fstream file;
	file.open(path, ios::out | ios::binary);
	if (file.is_open()) {
		// area
		file.write((char*)&m_tag, sizeof(AreaTag));
		// wind
		file.write((char*)&m_wind, sizeof(float3));
		// fruit spawns
		file.write((char*)m_fruitSpawn, sizeof(int) * NR_OF_FRUITS);
		// terrain
		storeToFile_binary(file);

		file.close();
	}
	else
		ErrorLogger::logError(
			"(Environment) Failed saving environment to file! path: " + path, HRESULT());
}

void Environment::imgui_properties() {
	Transformation::imgui_properties();
	// wind
	ImGui::InputFloat3("Wind", (float*)&m_wind);
	// area tag
	static AreaTag tag = m_tag;
	ImGui::SetNextItemWidth(100);
	if (ImGui::BeginCombo("AreaTag", AreaTagToString(tag).c_str())) {
		for (size_t i = 0; i < AreaTag::NR_OF_AREAS; i++) {
			if (ImGui::MenuItem(AreaTagToString((AreaTag)i).c_str())) {
				tag = (AreaTag)i;
				setTag(tag);
			}
		}
		ImGui::EndCombo();
	}
	// fruit spawn
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		ImGui::SetNextItemWidth(100);
		if (ImGui::InputInt(("Spawn Fruit (" + FruitTypeToString((FruitType)i) + ")").c_str(),
				&m_fruitSpawn[i])) {
			m_fruitSpawn[i] = max(0, m_fruitSpawn[i]);
		}
	}
	// heightmap
	ImGui::Separator();
	static const TextureRepository::Type type = TextureRepository::Type::type_heightmap;
	static vector<shared_ptr<TextureSet>> heightmapTextures = {
		TextureRepository::get("flatmap.jpg", type), TextureRepository::get("DesertMap.png", type),
		TextureRepository::get("forestMap.png", type), TextureRepository::get("PlainMap.png", type),
		TextureRepository::get("tutorial.png", type), TextureRepository::get("VolcanoMap.png", type)
	};
	static string heightmap = heightmapTextures[0]->filename;
	ImGui::SetNextItemWidth(150);
	if (ImGui::BeginCombo("Heightmap", heightmap.c_str())) {
		float cWidth = ImGui::CalcItemWidth();
		int itemCountOnWidth = 3;
		for (size_t i = 0; i < heightmapTextures.size(); i++) {
			ImGui::BeginGroup();
			ImGui::Text(heightmapTextures[i]->filename.c_str());
			if (ImGui::ImageButton(heightmapTextures[i]->view.Get(),
					ImVec2(cWidth / itemCountOnWidth, cWidth / itemCountOnWidth)))
				heightmap = heightmapTextures[i]->filename;
			ImGui::EndGroup();
			if ((i + 1) % itemCountOnWidth != 0)
				ImGui::SameLine();
		}
		ImGui::EndCombo();
	}
	// subsize / tile size
	static XMINT2 subSize = getSubSize();
	ImGui::Text("Sub Size: (%i %i)", getSubSize().x, getSubSize().y);
	ImGui::SetNextItemWidth(100);
	if (ImGui::InputInt2("##", (int*)&subSize)) {
		subSize = XMINT2(max(0, subSize.x), max(0, subSize.y));
	}
	// divisions / grid size
	static XMINT2 divisions = getSplits();
	ImGui::Text("Divisions: (%i %i)", getSplits().x, getSplits().y);
	ImGui::SetNextItemWidth(100);
	if (ImGui::InputInt2("###", (int*)&divisions)) {
		divisions = XMINT2(max(0, divisions.x), max(0, divisions.y));
	}
	ImGui::Text(
		"Triangles: %i", getSubSize().x * getSubSize().y * getSplits().x * getSplits().y * 2);
	// buttons
	if (ImGui::Button("Rebuild")) {
		build(heightmap, subSize, divisions);
	}
	ImGui::SameLine();
	if (ImGui::Button("Resize")) {
		changeSize(subSize, divisions);
	}
	ImGui::Separator();
}
