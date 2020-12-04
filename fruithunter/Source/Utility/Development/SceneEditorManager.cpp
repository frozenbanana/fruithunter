#include "SceneEditorManager.h"
#include "Renderer.h"
#include "AudioController.h"
#include "PathFindingThread.h"
#include "Settings.h"
#include <WICTextureLoader.h>
#include <ScreenGrab.h>
#include <wincodec.h>

void SceneEditorManager::update_imgui_library() {
	Input* ip = Input::getInstance();
	{
		if (ImGui::BeginChild("win_fragmentDisplayer", ImVec2(200, 0), true, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImVec2 childSize = ImGui::GetWindowSize();
			ImGui::Text("Fragments");
			if (ImGui::ListBoxHeader("", ImVec2(200, childSize.y-125))) {
				for (size_t i = 0; i < m_library.size(); i++) {
					bool selected = (m_selectedIndex == i);
					if (ImGui::Selectable(m_library[i]->getFullDescription().c_str(), selected)) {
						if (selected) {
							deselect_fragment();
						}
						else {
							select_fragment(i);
						}
					}
				}
				ImGui::ListBoxFooter();
			}
			if (m_selectedIndex != -1) {
				Fragment* f = m_library[m_selectedIndex];
				if (ImGui::Button("Find")) {
					Transformation* t = dynamic_cast<Transformation*>(f);
					if (t != nullptr) {
						m_camera.setEye(t->getPosition() + float3(1, 1, 0) * 2.f);
						m_camera.setTarget(t->getPosition());
					}
				}
				ImGui::SameLine();
				if (ImGui::Button("Remove") || ip->keyPressed(m_key_delete)) {
					scene->remove_fragment(f);
					refreshLibrary();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("QuickButton (Del)");
					ImGui::EndTooltip();
				}
				ImGui::SameLine();
				if (ImGui::Button("Copy") || ip->keyPressed(m_key_copy)) {
					Fragment::Type type = f->getType();
					bool anyUpdate = true;
					if (type == Fragment::Type::animal) {
						Animal* obj = dynamic_cast<Animal*>(f);
						scene->m_animals.push_back(make_shared<Animal>(*obj));
					}
					else if (type == Fragment::Type::entity) {
						Entity* target = dynamic_cast<Entity*>(f);
						shared_ptr<Entity> obj = make_shared<Entity>(*target);
						scene->m_entities.add(obj->getLocalBoundingBoxPosition(),
							obj->getLocalBoundingBoxSize(), obj->getMatrix(), obj);
					}
					else if (type == Fragment::Type::particleSystem) {
						ParticleSystem* obj = dynamic_cast<ParticleSystem*>(f);
						scene->m_particleSystems.push_back(*obj);
					}
					else if (type == Fragment::Type::sea) {
						SeaEffect* target = dynamic_cast<SeaEffect*>(f);
						shared_ptr<SeaEffect> obj = make_shared<SeaEffect>(*target);
						scene->m_seaEffects.push_back(obj);
					}
					else if (type == Fragment::Type::terrain) {
						Environment* obj = dynamic_cast<Environment*>(f);
						shared_ptr<Environment> sh_obj = make_shared<Environment>(*obj);
						scene->m_terrains.add(sh_obj);
					}
					else
						anyUpdate = false;
					if (anyUpdate)
						refreshLibrary();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::Text("QuickButton (C)");
					ImGui::EndTooltip();
				}
			}

			// help text
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1,3));
			ImGui::Text(" --- Buttons ---");
			ImVec4 btnCol(0, 1, 0, 1);
			ImGui::Text("Select object: ");
			ImGui::SameLine();
			ImGui::TextColored(btnCol, "MiddleClick");
			ImGui::Text("Place pointer: ");
			ImGui::SameLine();
			ImGui::TextColored(btnCol, "RightClick");
			ImGui::Text("Switch transform edit: ");
			ImGui::SameLine();
			ImGui::TextColored(btnCol, "Tab");
			ImGui::PopStyleVar();

			ImGui::EndChild();
		}
	}
	ImGui::SameLine();
	{
		ImGui::PushItemWidth(150);
		ImGui::BeginGroup();
		if (m_selectedIndex != -1) {
			bool updated = false;
			Fragment* f = m_library[m_selectedIndex];
			switch (f->getType()) {
			case Fragment::entity:
				updated = update_panel_entity(dynamic_cast<Entity*>(f), m_selectedThisFrame);
				break;
			case Fragment::animal:
				updated = update_panel_animal(dynamic_cast<Animal*>(f), m_selectedThisFrame);
				break;
			case Fragment::terrain:
				updated = update_panel_terrain(dynamic_cast<Environment*>(f), m_selectedThisFrame);
				break;
			case Fragment::particleSystem:
				updated =
					update_panel_effect(dynamic_cast<ParticleSystem*>(f), m_selectedThisFrame);
				break;
			case Fragment::sea:
				updated = update_panel_sea(dynamic_cast<SeaEffect*>(f), m_selectedThisFrame);
				break;
			}
			if (updated)
				scene->updated_fragment(f);
		}
		else {
			bool updated = false; // tells if a new fragment has been created
			if (ImGui::BeginTabBar("fragmentCreator")) {
				if (ImGui::BeginTabItem("Terrain")) {
					updated = update_panel_terrain(nullptr);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Entity")) {
					updated = update_panel_entity(nullptr);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Animal")) {
					updated = update_panel_animal(nullptr);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("SeaEffect")) {
					updated = update_panel_sea(nullptr);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("ParticleSystem")) {
					updated = update_panel_effect(nullptr);
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			if (updated)
				refreshLibrary();
		}
		ImGui::EndGroup();
		ImGui::PopItemWidth();
	}
}

void SceneEditorManager::update_imgui_gameRules() {
	static char text[50];
	memset(text, NULL, 50);
	memcpy(text, scene->m_sceneName.c_str(), scene->m_sceneName.size());
	ImGui::SetNextItemWidth(100);
	if (ImGui::InputText("Scene Name", text, 50)) {
		scene->m_sceneName = string(text);
	}
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		ImGui::SetNextItemWidth(100);
		ImGui::InputInt(("WinCondition (" + FruitTypeToString((FruitType)i) + ")").c_str(),
			&scene->m_utility.winCondition[i]);
	}
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		ImGui::SetNextItemWidth(100);
		ImGui::InputInt(("Time Target (" + TimeTargetToString((TimeTargets)i) + ")").c_str(),
			&scene->m_utility.timeTargets[i]);
	}
	if (ImGui::Button("FromPoint"))
		scene->m_utility.startSpawn = m_pointer;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	ImGui::InputFloat3("Start Spawn", (float*)&scene->m_utility.startSpawn);
}

void SceneEditorManager::update_imgui_terrainEditor() {
	ImGui::SetNextItemWidth(200);
	ImGui::SliderFloat("Radius", &m_terrainBrush.radius, 0.1, 10);
	static float falloffPoints[25] = {-1};
	ImGui::SetNextItemWidth(200);
	if (ImGui::SliderFloat("Falloff", &m_terrainBrush.falloff, 0, 5) || falloffPoints[0] == -1) {
		for (size_t i = 0; i < ARRAYSIZE(falloffPoints); i++) {
			float x = (float)i / (ARRAYSIZE(falloffPoints) - 1);
			falloffPoints[i] =
				1 - pow(1 - 0.5 * (1 - cos(x * 3.1415f)),
						1.f / m_terrainBrush.falloff);
		}
	}
	ImGui::PlotLines(
		"Falloff Visual", falloffPoints, ARRAYSIZE(falloffPoints), 0, NULL, 0, 1, ImVec2(200, 40));
	ImGui::SetNextItemWidth(200);
	ImGui::SliderFloat("Strength", &m_terrainBrush.strength, 0, 1);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 3));
	ImGui::Text(" --- Buttons ---");
	ImVec4 btnCol(0, 1, 0, 1);
	ImGui::Text("Raise: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "LeftClick");
	ImGui::Text("Lower: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "RightClick");
	ImGui::Text("Flatten: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "MiddleClick");
	ImGui::Text("Undo edit: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "Z");
	ImGui::Text("Change radius: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "ScrollWheel");
	ImGui::Text("Change strength: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "Shift + ScrollWheel");
	ImGui::PopStyleVar();
}

bool SceneEditorManager::update_panel_terrain(Environment* selection, bool update) {
	bool isValid = (selection != nullptr);
	bool updated = false;
	static float3 position, rotation, scale = float3(100, 15, 100);
	static AreaTag tag = AreaTag::Forest;
	static string heightmap = m_heightmap_textures[0]->filename;
	static string textures[4] = { m_terrain_textures[0]->filename, m_terrain_textures[0]->filename,
		m_terrain_textures[7]->filename, m_terrain_textures[7]->filename };
	static XMINT2 subSize = XMINT2(15, 15);
	static XMINT2 divisions = XMINT2(16, 16);
	static float3 wind;
	static int fruitSpawns[NR_OF_FRUITS] = { 0 };

	if (update && isValid) {
		position = selection->getPosition();
		rotation = selection->getRotation();
		scale = selection->getScale();
		tag = selection->getTag();
		//heightmap = selection->getLoadedHeightmapFilename();
		selection->getTextures(textures);
		subSize = selection->getSubSize();
		divisions = selection->getSplits();
		wind = selection->getWindStatic();
		size_t index = scene->find_parentIndex(selection);
		for (size_t i = 0; i < NR_OF_FRUITS; i++)
			fruitSpawns[i] = selection->getFruitCount((FruitType)i);
	}

	if (ImGui::InputFloat3("Position", (float*)&position, 2) && isValid) {
		selection->setPosition(position);
		updated = true;
	}
	if (ImGui::InputFloat3("Rotation", (float*)&rotation, 2) && isValid) {
		selection->setRotation(rotation);
		updated = true;
	}
	if (ImGui::InputFloat3("Scale", (float*)&scale, 2) && isValid) {
		selection->setScale(scale);
		updated = true;
	}
	if (ImGui::InputInt2("SubSize", (int*)&subSize)) {
		subSize = XMINT2(Clamp<int>(subSize.x,0,subSize.x),Clamp<int>(subSize.y,0,subSize.y));
	}
	if (ImGui::InputInt2("Divisions", (int*)&divisions)) {
		divisions = XMINT2(
			Clamp<int>(divisions.x, 0, divisions.x), Clamp<int>(divisions.y, 0, divisions.y));
	}
	if (ImGui::InputFloat3("Wind", (float*)&wind) && isValid) {
		selection->setWind(wind);
		updated = true;
	}
	if (ImGui::BeginCombo("AreaTag", AreaTagToString(tag).c_str())) {
		for (size_t i = 0; i < AreaTag::NR_OF_AREAS; i++) {
			if (ImGui::MenuItem(AreaTagToString((AreaTag)i).c_str())) {
				tag = (AreaTag)i;
				if (isValid) {
					selection->setTag(tag);
					updated = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Heightmap", heightmap.c_str())) {
		float cWidth = ImGui::CalcItemWidth();
		int itemCountOnWidth = 3;
		for (size_t i = 0; i < m_heightmap_textures.size(); i++) {
			ImGui::BeginGroup();
			ImGui::Text(m_heightmap_textures[i]->filename.c_str());
			if (ImGui::ImageButton(m_heightmap_textures[i]->view.Get(),
					ImVec2(cWidth / itemCountOnWidth, cWidth / itemCountOnWidth)))
				heightmap = m_heightmap_textures[i]->filename;
			ImGui::EndGroup();
			if ((i + 1) % itemCountOnWidth != 0)
				ImGui::SameLine();
		}
		ImGui::EndCombo();
	}
	static const string tex_description[4] = { "Texture Flat", "Texture Bottom Flat", "Texture Steep",
		"Texture Minimal Steep" };
	for (size_t i = 0; i < 4; i++) {
		if (ImGui::BeginCombo(tex_description[i].c_str(), textures[i].c_str())) {
			float cWidth = ImGui::CalcItemWidth();
			int itemCountOnWidth = 3;
			for (size_t j = 0; j < m_terrain_textures.size(); j++) {
				ImGui::BeginGroup();
				ImGui::Text(m_terrain_textures[j]->filename.c_str());
				if (ImGui::ImageButton(m_terrain_textures[j]->view.Get(),
						ImVec2(cWidth / itemCountOnWidth, cWidth / itemCountOnWidth))) {
					textures[i] = m_terrain_textures[j]->filename;
					if (isValid) {
						selection->setTextures(textures);
						updated = true;
					}
				}
				ImGui::EndGroup();
				if ((j + 1) % itemCountOnWidth != 0)
					ImGui::SameLine();
			}
			ImGui::EndCombo();
		}
	}
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		if (ImGui::InputInt(("Spawn Fruit (" + FruitTypeToString((FruitType)i) + ")").c_str(),
				&fruitSpawns[i]) && isValid) {
			fruitSpawns[i] = Clamp<int>(fruitSpawns[i], 0, fruitSpawns[i]);
			size_t index = scene->find_parentIndex(selection);
			scene->m_terrains.getTerrainFromIndex(index)->setFruitSpawns(fruitSpawns);
		}
	}
	if (!isValid) {
		if (ImGui::Button("Create")) {
			updated = true;
			scene->m_terrains.add(
				position, scale, heightmap, textures, subSize, divisions, wind, tag);
		}
	} else {
		if (ImGui::Button("Rebuild")) {
			selection->build(heightmap, subSize, divisions);
			updated = true;
		}
		ImGui::SameLine();
		if (ImGui::Button("Resize")) {
			selection->changeSize(subSize, divisions);
			updated = true;
		}
	}
	return updated;
}

bool SceneEditorManager::update_panel_entity(Entity* selection, bool update) {
	bool updated = false;
	bool isValid = (selection != nullptr);
	static float3 position, rotation, scale = float3(1.);
	static string mesh = m_loadable_entity[0];
	static bool collision = true;

	if (update && isValid) {
		position = selection->getPosition();
		rotation = selection->getRotation();
		scale = selection->getScale();
		mesh = selection->getModelName();
		collision = selection->getIsCollidable();
	}

	if (ImGui::BeginCombo("Meshes", mesh.c_str())) {
		float cWidth = ImGui::CalcItemWidth();
		int itemCountOnWidth = 3;
		for (size_t i = 0; i < m_loadable_entity.size(); i++) {
			if (ImGui::MenuItem(m_loadable_entity[i].c_str())) {
				mesh = m_loadable_entity[i];
				if (isValid) {
					selection->load(mesh);
					updated = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::Button("Point")) {
		position = m_pointer;
		if (isValid) {
			selection->setPosition(position);
			updated = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::InputFloat3("Position", (float*)&position) && isValid) {
		selection->setPosition(position);
		updated = true;
	}
	if (ImGui::InputFloat3("Rotation", (float*)&rotation) && isValid) {
		selection->setRotation(rotation);
		updated = true;
	}
	if (ImGui::InputFloat3("Scale", (float*)&scale) && isValid) {
		selection->setScale(scale);
		updated = true;
	}
	if (ImGui::Checkbox("Collidable", &collision) && isValid) {
		selection->setCollidable(collision);
		updated = true;
	}
	if (!isValid) {
		if (ImGui::Button("Create")) {
			updated = true;
			shared_ptr<Entity> e = make_shared<Entity>(mesh, m_pointer, scale);
			e->setRotation(rotation);
			e->setCollidable(collision);
			scene->m_entities.add(
				e->getLocalBoundingBoxPosition(), e->getLocalBoundingBoxSize(), e->getMatrix(), e);
		}
	}
	return updated;
}

bool SceneEditorManager::update_panel_animal(Animal* selection, bool update) {
	bool updated = false;
	bool isValid = (selection != nullptr);

	static const string types[Animal::Type::Length] = { "Bear", "Goat", "Gorilla" };
	static float3 position, sleepPosition;
	static float rotationY = 0;
	static Animal::Type type = Animal::Type::Bear;
	static FruitType fruitType = FruitType::APPLE;
	static int fruitCount = 1;

	if (update && isValid) {
		position = selection->getPosition();
		rotationY = selection->getRotation().y;
		type = selection->getType();
		fruitType = selection->getfruitType();
		fruitCount = selection->getRequiredFruitCount();
		sleepPosition = selection->getSleepPosition();
	}
	if (ImGui::Button("UpdateP")) {
		position = m_pointer;
		if (isValid) {
			selection->setPosition(position);
			updated = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::InputFloat3("Position", (float*)&position, 2) && isValid) {
		selection->setPosition(position);
		updated = true;
	}
	if (ImGui::InputFloat("Rotation", (float*)&rotationY, 2) && isValid) {
		selection->setRotation(float3(0, rotationY, 0));
		updated = true;
	}
	if (ImGui::Button("UpdateSP")) {
		sleepPosition = m_pointer;
		if (isValid) {
			selection->setSleepPosition(sleepPosition);
			updated = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::InputFloat3("Sleep Position", (float*)&sleepPosition, 2) && isValid) {
		selection->setSleepPosition(sleepPosition);
		updated = true;
	}
	if (ImGui::BeginCombo("Model", types[type].c_str())) {
		for (size_t i = 0; i < Animal::Type::Length; i++) {
			if (ImGui::MenuItem(types[i].c_str())) {
				type = (Animal::Type)i;
				if (isValid) {
					selection->setType(type);
					updated = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::BeginCombo("Fruit To Give", FruitTypeToString(fruitType).c_str())) {
		for (size_t i = 0; i < FruitType::NR_OF_FRUITS; i++) {
			if (ImGui::MenuItem(FruitTypeToString((FruitType)i).c_str())) {
				fruitType = (FruitType)i;
				if (isValid) {
					selection->setFruitType(fruitType);
					updated = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::InputInt("Fruit Count", &fruitCount)) {
		fruitCount = Clamp<int>(fruitCount, 0, 3);
		if (isValid) {
			selection->setRequiredFruitCount(fruitCount);
			updated = true;
		}
	}
	if (!isValid) {
		if (ImGui::Button("Create")) {
			updated = true;
			scene->m_animals.push_back(make_shared<
				Animal>(position, sleepPosition, type, fruitType, fruitCount, rotationY));
		}
	}
	return updated;
}

bool SceneEditorManager::update_panel_sea(SeaEffect* selection, bool update) {
	bool updated = false;
	bool isValid = (selection != nullptr);

	static float3 position, rotation, scale = float3(1.);
	static XMINT2 tiles = XMINT2(1, 1), grids = XMINT2(1, 1);
	static SeaEffect::SeaEffectTypes type = SeaEffect::SeaEffectTypes::water;
	static string typeAsString[SeaEffect::SeaEffectTypes::Count] = { "Water", "Lava" };
	static float color[3];
	if (update && isValid) {
		position = selection->getPosition();
		rotation = selection->getRotation();
		scale = selection->getScale();
		type = selection->getType();
		tiles = selection->getTileSize();
		grids = selection->getGridSize();
	}
	if (ImGui::InputFloat3("Position", (float*)&position) && isValid) {
		selection->setPosition(position);
		updated = true;
	}
	if (ImGui::InputFloat3("Rotation", (float*)&rotation) && isValid) {
		selection->setRotation(rotation);
		updated = true;
	}
	if (ImGui::InputFloat3("Scale", (float*)&scale) && isValid) {
		selection->setScale(scale);
		updated = true;
	}
	if (ImGui::BeginCombo("Type", typeAsString[type].c_str())) {
		for (size_t i = 0; i < SeaEffect::SeaEffectTypes::Count; i++) {
			if (ImGui::MenuItem(typeAsString[i].c_str())) {
				type = (SeaEffect::SeaEffectTypes)i;
				if (isValid) {
					selection->setType(type);
					updated = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::InputInt2("Tiles", (int*)&tiles))
		tiles = XMINT2(Clamp<int>(tiles.x, 0, tiles.x), Clamp<int>(tiles.y, 0, tiles.y));
	if(ImGui::InputInt2("Grids", (int*)&grids))
		grids = XMINT2(Clamp<int>(grids.x, 0, grids.x), Clamp<int>(grids.y, 0, grids.y));
	if (!isValid) {
		if (ImGui::Button("Create")) {
			updated = true;
			shared_ptr<SeaEffect> se = make_shared<SeaEffect>();
			se->initilize(type, tiles, grids, position, scale, rotation);
			scene->m_seaEffects.push_back(se);
		}
	} else {
		if (ImGui::Button("Rebuild")) {
			size_t index = scene->find_parentIndex(selection);
			scene->m_seaEffects[index]->build(tiles, grids);
			updated = true;
		}
	}
	return updated;
}

bool SceneEditorManager::update_panel_effect(ParticleSystem* selection, bool update) {
	bool updated = false;
	bool isValid = (selection != nullptr);

	static int shape = 0;
	static ParticleSystem::ParticleDescription pd;
	static ParticleSystem::Type type = ParticleSystem::Type::CONFETTI;
	static float3 position, size;
	static int emitRate = 0;
	static int capacity = 0;
	static bool affectedByWind = false;
	static int emitCount = 1;
	if (update && isValid) {
		type = selection->getType();
		position = selection->getPosition();
		size = selection->getScale();
		emitRate = selection->getEmitRate();
		capacity = selection->getCapacity();
		affectedByWind = selection->isAffectedByWind();
		pd = selection->getParticleDescription();
	}
	if (ImGui::Button("UpdateP")) {
		position = m_pointer;
		if (isValid) {
			selection->setPosition(position);
			updated = true;
		}
	}
	ImGui::SameLine();
	if (ImGui::InputFloat3("Position", (float*)&position, 2) && isValid) {
		selection->setPosition(position);
		updated = true;
	}
	if (ImGui::InputFloat3("Size", (float*)&size, 2)) {
		size = float3(abs(size.x), abs(size.y), abs(size.z));
		if (isValid) {
			selection->setScale(size);
			updated = true;
		}
	}
	static string ps_typeAsString[ParticleSystem::Type::TYPE_LENGTH] = { "None", "Forest Bubble",
		"Ground Dust", "Volcano Fire", "Volcano Smoke", "Lava Bubble", "Arrow Glitter", "Confetti",
		"Stars Gold", "Stars Silver", "Stars Bronze", "Explosion Apple", "Explosion Banana",
		"Explosion Melon", "Explosion Dragon", "Sparkle Apple", "Sparkle Banana", "Sparkle Melon",
		"Sparkle Dragon", "Explosion Gold", "Explosion Silver", "Explosion Bronze", "Jump Dust" };
	if (ImGui::BeginCombo("Type", ps_typeAsString[type].c_str())) {
		for (size_t i = 1; i < ParticleSystem::Type::TYPE_LENGTH; i++) {
			if (ImGui::MenuItem(ps_typeAsString[i].c_str())) {
				type = (ParticleSystem::Type)i;
				if (isValid) {
					selection->setType(type);
					updated = true;
				}
			}
		}
		ImGui::EndCombo();
	}
	if (ImGui::InputInt("Emit Rate", &emitRate)) {
		emitRate = Clamp<int>(emitRate, 0, emitRate);
		if (isValid) {
			selection->setEmitRate(emitRate);
			updated = true;
		}
	}
	if (ImGui::InputInt("Capacity", &capacity)) {
		capacity = Clamp<int>(capacity, 0, capacity);
		if (isValid) {
			selection->setCapacity(capacity);
			updated = true;
		}
	}
	if (ImGui::Checkbox("Wind", &affectedByWind) && isValid) {
		selection->affectedByWindState(affectedByWind);
		updated = true;
	}
	if (!isValid) {
		if (ImGui::Button("Create")) {
			updated = true;
			ParticleSystem ps;
			ps.load(type, 10, 0);
			ps.setEmitRate(emitRate, false);
			ps.setCapacity(capacity);
			ps.affectedByWindState(affectedByWind);
			ps.setPosition(position);
			ps.setScale(size);
			scene->m_particleSystems.push_back(ps);
		}
	} else {
		if (ImGui::Button("Emit")) {
			selection->emit(emitCount);
		}
		ImGui::SameLine();
		ImGui::InputInt("Count", &emitCount, 1);
	}
	//Custom testing
	if (isValid) {
		ImGui::Separator();
		ImGui::Text("CUSTOM TESTING (Temporary panel)");
		ImGui::ColorEdit4("Color0", (float*)&pd.colorVariety[0]);
		ImGui::ColorEdit4("Color1", (float*)&pd.colorVariety[1]);
		ImGui::ColorEdit4("Color2", (float*)&pd.colorVariety[2]);
		ImGui::InputFloat2("Size Interval", (float*)&pd.size_interval);
		ImGui::InputFloat2("TimeAlive Interval", (float*)&pd.timeAlive_interval);
		ImGui::InputFloat3("Velocity min", (float*)&pd.velocity_min);
		ImGui::InputFloat3("Velocity max", (float*)&pd.velocity_max);
		ImGui::InputFloat2("Velocity Interval", (float*)&pd.velocity_interval);
		ImGui::InputFloat3("Acceleration", (float*)&pd.acceleration);
		ImGui::InputFloat("Slowdown", &pd.slowdown, 0, 0, 6);
		if (ImGui::Combo("Shape", &shape, "Circle\0Star"))
			pd.shape = (ParticleSystem::ParticleDescription::Shape)shape;
		if (ImGui::Button("Set"))
			selection->setCustomDescription(pd);
	}
	return updated;
}

void SceneEditorManager::refreshLibrary() {
	m_library.clear();
	m_library.reserve(scene->m_terrains.length() + scene->m_seaEffects.size() +
					  scene->m_entities.size() + scene->m_particleSystems.size() +
					  scene->m_animals.size());
	for (size_t i = 0; i < scene->m_terrains.length(); i++)
		m_library.push_back(scene->m_terrains.getTerrainFromIndex(i).get());
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++)
		m_library.push_back(scene->m_seaEffects[i].get());
	for (size_t i = 0; i < scene->m_entities.size(); i++)
		m_library.push_back(scene->m_entities[i].get());
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++)
		m_library.push_back(&scene->m_particleSystems[i]);
	for (size_t i = 0; i < scene->m_animals.size(); i++)
		m_library.push_back(scene->m_animals[i].get());

	if (m_library.size() == 0) {
		deselect_fragment();
	}
	else if (m_selectedIndex != -1) {
		m_selectedIndex = Clamp<size_t>(m_selectedIndex, 0, m_library.size() - 1);
		select_fragment(m_selectedIndex);
	}
}

void SceneEditorManager::updateCameraMovement(float dt) {
	Input* ip = Input::getInstance();
	// Position
	const float3 STD_Forward = float3(0.0f, 0.0f, 1.0f);
	float3 forward = m_camera.getForward();
	float3 up = m_camera.getUp();
	float3 right = m_camera.getRight();
	float3 acceleration = forward * (float)(ip->keyDown(KEY_FORWARD) - ip->keyDown(KEY_BACKWARD)) + 
		right * (float)(ip->keyDown(KEY_RIGHT) - ip->keyDown(KEY_LEFT)) +
		up*(float)(ip->keyDown(KEY_UP) - ip->keyDown(KEY_DOWN));
	acceleration.Normalize();
	float speed = ip->keyDown(KEY_SLOW) ? m_lowSpeed : m_highSpeed;
	acceleration *= speed;

	m_cam_velocity += acceleration * dt;
	m_camera.move(m_cam_velocity * dt);
	m_cam_velocity *= pow(m_cam_friction / 60.f, dt); // friction/slowdown
	// Rotation
	float2 mouseMovement;
	if (ip->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		mouseMovement = float2((float)ip->mouseY(), (float)ip->mouseX());
	}
	float rotationSpeed = (0.001) + Settings::getInstance()->getSensitivity() * dt;
	mouseMovement *= rotationSpeed;
	m_camera.rotate(float3(mouseMovement.x, mouseMovement.y, 0));
}

void SceneEditorManager::update_transformation(float dt) {
	Input* ip = Input::getInstance();

	// Camera properties
	float3 point = m_camera.getPosition();
	float3 direction = m_camera.getForward();

	// Handle transformation
	if (m_transformable != nullptr) {
		// target transform option
		if (ip->mousePressed(m_key_target)) {
			// ray cast
			if (m_transformState == Edit_Translate) {
				float t_min = -1;
				for (size_t i = 0; i < 3; i++) {
					float t = m_arrow[i].castRay(point, direction);
					if (t != -1 && (t_min == -1 || t < t_min)) {
						t_min = t;
						m_target = i;
					}
				}
				float t = m_centerOrb.castRay(point, direction);
				if (t != -1 && (t_min == -1 || t < t_min)) {
					t_min = t;
					m_target = 4; // center orb
					m_target_pos = m_camera.getPosition();
				}
				m_target_pos = m_camera.getPosition();
				m_target_rot = m_camera.getRotation();
				m_target_forward = m_camera.getForward();
				m_target_rayDist = t_min;
			}
			else if (m_transformState == Edit_Rotation) {
				float t_min = -1;
				for (size_t i = 0; i < 3; i++) {
					float t = m_torus[i].castRay(point, direction);
					if (t != -1 && (t_min == -1 || t < t_min)) {
						t_min = t;
						m_target = i;
					}
				}
				m_target_pos = m_camera.getPosition();
				m_target_rot = m_camera.getRotation();
				m_target_forward = m_camera.getForward();
				m_target_rayDist = t_min;
			}
			else if (m_transformState == Edit_Scaling) {
				float t = m_scaling_torus.castRay(point, direction);
				if (t != -1) {
					m_target = 4; // center orb
					m_target_pos = m_camera.getPosition();
				}
				m_target_pos = m_camera.getPosition();
				m_target_rot = m_camera.getRotation();
				m_target_forward = m_camera.getForward();
				m_target_rayDist = t;
			}
		}
		else if (ip->mouseDown(m_key_target) && m_target != -1) {
			float3 posDiff = m_camera.getPosition() - m_target_pos;
			float3 rotDiff = m_camera.getRotation() - m_target_rot;
			float3 forwardDiff = m_camera.getForward() - m_target_forward;
			float3 target_forward = m_target_forward;
			float3 target_pos = m_target_pos;
			m_target_pos = m_camera.getPosition();
			m_target_rot = m_camera.getRotation();
			m_target_forward = m_camera.getForward();
			if (m_transformState == Edit_Translate) {
				if (m_target == 4) { // center orb
					// change from position
					m_transformable->move(posDiff);
					// change from rotation
					m_transformable->move(forwardDiff * m_target_rayDist);
				}
				else { // arrow
					float3 transformPosition = m_transformable->getPosition();
					float3 toTransform = transformPosition - target_pos;
					float3 normToTarget = target_forward;
					float3 axis = m_axis[m_target];
					float3 p_up = Normalize(toTransform.Cross(axis));
					float3 p_n = Normalize(p_up.Cross(axis));

					float3 p1 =
						target_pos + target_forward * RayPlaneIntersection(target_pos,
														  target_forward, transformPosition, p_n);
					float3 p2 = m_target_pos + m_target_forward * RayPlaneIntersection(m_target_pos,
																	  m_target_forward,
																	  transformPosition, p_n);

					m_transformable->move(axis * axis.Dot(p2 - p1));
				}
			}
			else if (m_transformState == Edit_Rotation) {
				float3 transformPosition = m_transformable->getPosition();
				float3 stdAxis = m_axis[m_target];
				float3 axis = float3::Transform(
					stdAxis, CreatePYRMatrix(m_transformable->getRotation() * m_maskPYR[m_target]));
				float3 p1 =
					target_pos + target_forward * RayPlaneIntersection(target_pos, target_forward,
													  transformPosition, axis);
				float3 p2 = m_target_pos + m_target_forward * RayPlaneIntersection(m_target_pos,
																  m_target_forward,
																  transformPosition, axis);
				float3 toP1 = Normalize(p1 - transformPosition);
				float3 toP2 = Normalize(p2 - transformPosition);
				float rad = acos(Clamp(toP1.Dot(toP2), -1.f, 1.f));
				float sign = 1;
				if ((toP1.Cross(toP2 - toP1) - axis).Length() > 1.0f)
					sign = -1;
				m_transformable->rotate(stdAxis * rad * sign);
			}
			else if (m_transformState == Edit_Scaling) {
				float3 oldPoint = target_pos + target_forward * m_target_rayDist;
				float3 point = m_camera.getPosition() + m_camera.getForward() * m_target_rayDist;
				float3 pointDiff = point - oldPoint;
				float scale = (m_transformable->getPosition() - point).Length() /
							  (m_transformable->getPosition() - oldPoint).Length();
				m_transformable->rescale(scale);
			}
			// if entity, then need to update quadtree
			Fragment* f = dynamic_cast<Fragment*>(m_transformable);
			if (f != nullptr)
				scene->updated_fragment(f);
		}
		else {
			m_target = -1;
		}
		// switch Transform state
		if (ip->keyPressed(m_key_switchState)) {
			m_transformState =
				(EditTransformState)((m_transformState + 1) % EditTransformState::Edit_Count);
			string strState[3] = { "Translate", "Scaling", "Rotation" };
			cout << "Switch TransformState -> " << strState[m_transformState] << endl;
		}
	}
}

void SceneEditorManager::draw_transformationVisuals() {
	if (m_transformable != nullptr) {
		float scale1 = m_transformable->getScale().Length();
		scale1 = Clamp<float>(scale1, 1, 10);
		float3 scale(scale1, scale1, scale1);
		if (m_transformState == Edit_Translate) {
			// arrows
			for (size_t i = 0; i < 3; i++) {
				m_arrow[i].setPosition(m_transformable->getPosition());
				m_arrow[i].setScale(scale);
				m_arrow[i].draw_onlyMesh(m_axis[i]);
			}
			m_centerOrb.setPosition(m_transformable->getPosition());
			m_centerOrb.setScale(scale * 0.25f);
			m_centerOrb.draw_onlyMesh(float3(1.));
		}
		else if (m_transformState == Edit_Rotation) {
			for (size_t i = 0; i < 3; i++) {
				m_torus[i].setPosition(m_transformable->getPosition());
				m_torus[i].setRotation(m_transformable->getRotation() * m_maskPYR[i]);
				m_torus[i].setScale(scale);
				m_torus[i].draw_onlyMesh(m_axis[i]);
			}
		}
		else if (m_transformState == Edit_Scaling) {
			float cubeScaling = 1;
			m_scaling_torus.setPosition(m_transformable->getPosition());
			m_scaling_torus.setScale(cubeScaling * scale);
			m_scaling_torus.draw_onlyMesh(float3(1.));
		}
	}
}

void SceneEditorManager::update_imgui() {
	Input* ip = Input::getInstance();

	// Main Menu Bar
	ImVec2 menuBarSize;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::BeginMenu("Load")) {
				for (size_t i = 0; i < m_loadable_scenes.size(); i++) {
					if (ImGui::MenuItem(m_loadable_scenes[i].c_str())) {
						load(m_loadable_scenes[i]);
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save")) {
				scene->save();
				readSceneDirectory();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Show")) {
			if (ImGui::MenuItem("Empty"))
				;
			ImGui::EndMenu();
		}
		menuBarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
	// Docked window
	float windHeight = Renderer::getInstance()->getScreenHeight() - menuBarSize.y;
	ImGui::SetNextWindowPos(ImVec2(0,menuBarSize.y));
	ImGui::SetNextWindowSizeConstraints(
		ImVec2(0, windHeight), ImVec2(Renderer::getInstance()->getScreenWidth(), windHeight));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (ImGui::Begin("Docked Window", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
		if (ImGui::BeginTabBar("editorContent")) {
			if (ImGui::BeginTabItem("Library")) {
				m_editorTabActive = EditorTab::Library;
				update_imgui_library();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Game Rules")) {
				m_editorTabActive = EditorTab::GameRules;
				update_imgui_gameRules();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Terrain Editor")) {
				m_editorTabActive = EditorTab::TerrainEditor;
				update_imgui_terrainEditor();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::End();
	}
	ImGui::PopStyleVar();

	m_selectedThisFrame = false;
}

void SceneEditorManager::select_fragment(size_t index) { 
	m_selectedIndex = index;
	m_selectedThisFrame = true;

	static const Fragment::Type validTypes[3] = { Fragment::Type::animal, Fragment::Type::entity,
		Fragment::Type::particleSystem };
	bool found = false;
	for (size_t i = 0; i < 3; i++) {
		if (m_library[index]->getType() == validTypes[i]) {
			found = true;
			m_transformable = dynamic_cast<Transformation*>(m_library[index]);
			break;
		}
	}
	if (!found)
		m_transformable = nullptr;
}

void SceneEditorManager::deselect_fragment() {
	m_selectedIndex = -1;
	m_selectedThisFrame = false;
	m_transformable = nullptr;
}

void SceneEditorManager::readSceneDirectory() {
	vector<string> dirs;
	read_directory("assets/Scenes", dirs);
	m_loadable_scenes = vector<string>(dirs.begin() + 2, dirs.end());
}

SceneEditorManager::SceneEditorManager() { 
	TextureRepository* tr = TextureRepository::getInstance();
	// terrain heightmap textures
	TextureRepository::Type type = TextureRepository::Type::type_heightmap;
	m_heightmap_textures.push_back(tr->get("DesertMap.png", type));
	m_heightmap_textures.push_back(tr->get("flatmap.jpg", type));
	m_heightmap_textures.push_back(tr->get("ForestMap.png", type));
	m_heightmap_textures.push_back(tr->get("PlainMap.png", type));
	m_heightmap_textures.push_back(tr->get("tutorial.png", type));
	m_heightmap_textures.push_back(tr->get("VolcanoMap.png", type));

	// terrain textures
	type = TextureRepository::Type::type_texture;
	m_terrain_textures.push_back(tr->get("texture_grass.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_grass3.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_grass6.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_lava1.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_mossyRock.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_mud.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_rock2.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_rock4.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_rock6.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_rock8.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_rock9.png", type));
	m_terrain_textures.push_back(tr->get("texture_sand1.jpg", type));
	m_terrain_textures.push_back(tr->get("texture_sand3.jpg", type));

	// loadable entities
	m_loadable_entity.push_back("Block");
	m_loadable_entity.push_back("Bowl");
	m_loadable_entity.push_back("BowlContent1");
	m_loadable_entity.push_back("BowlContent2");
	m_loadable_entity.push_back("BowlContent3");
	m_loadable_entity.push_back("BurnedTree1");
	m_loadable_entity.push_back("BurnedTree2");
	m_loadable_entity.push_back("BurnedTree3");
	m_loadable_entity.push_back("bush1");
	m_loadable_entity.push_back("bush2");
	m_loadable_entity.push_back("DeadBush");
	m_loadable_entity.push_back("Grass1");
	m_loadable_entity.push_back("Grass2");
	m_loadable_entity.push_back("Grass3");
	m_loadable_entity.push_back("Grass4");
	m_loadable_entity.push_back("Quad");
	m_loadable_entity.push_back("RopeBridgeFloor");
	m_loadable_entity.push_back("RopeBridgeRailing1");
	m_loadable_entity.push_back("RopeBridgeRailing2");
	m_loadable_entity.push_back("SignHorizontal");
	m_loadable_entity.push_back("SignHorizontal2");
	m_loadable_entity.push_back("SignSlanted");
	m_loadable_entity.push_back("SignSlanted2");
	m_loadable_entity.push_back("Sphere");
	m_loadable_entity.push_back("stone1");
	m_loadable_entity.push_back("stone2");
	m_loadable_entity.push_back("stone3");
	m_loadable_entity.push_back("treeMedium1");
	m_loadable_entity.push_back("treeMedium2");
	m_loadable_entity.push_back("treeMedium3");
	m_loadable_entity.push_back("Cactus_tall");
	m_loadable_entity.push_back("Cactus_small");
	m_loadable_entity.push_back("mushroom1");
	m_loadable_entity.push_back("mushroom2");
	m_loadable_entity.push_back("mushroom3");
	m_loadable_entity.push_back("mushroom4");
	m_loadable_entity.push_back("flower1");
	m_loadable_entity.push_back("flower2");
	m_loadable_entity.push_back("flower3");
	m_loadable_entity.push_back("flower4");
	m_loadable_entity.push_back("totem");

	// pointer object
	m_pointer_obj.load("sphere");
	m_pointer_obj.setScale(0.1);

	// crosshair
	m_crosshair.load("crosshair.png");
	m_crosshair.set(float2(1280./2, 720./2), float2(1./10));
	m_crosshair.setAlignment();//center - center

	// translation
	for (size_t i = 0; i < 3; i++) {
		m_arrow[i].load("arrow");
	}
	m_arrow[0].rotateY(XM_PI/2.f);
	m_arrow[1].rotateX(XM_PI/2.f);
	m_centerOrb.load("sphere");
	//rotation
	m_torus[0].load("torusX");
	m_torus[1].load("torusY");
	m_torus[2].load("torusZ");
	m_rotationCircle.load("sphere");
	//scaling
	m_scaling_torus.load("torusY");

	readSceneDirectory();

}

void SceneEditorManager::update() {
	Input* ip = Input::getInstance();

	scene->m_timer.update();
	float dt = scene->m_timer.getDt();

	// update camera
	updateCameraMovement(dt);

	// Update Skybox
	scene->m_skyBox.update(dt);
	const Environment* activeEnvironment = scene->m_terrains.getTerrainFromPosition(m_camera.getPosition());
	if (activeEnvironment != nullptr) {
		AreaTag tag = activeEnvironment->getTag();
		scene->m_skyBox.switchLight(tag);
		if (!m_manualCamera) {
			scene->update_activeTerrain(tag);
		}
	}

	// update water
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i]->update(dt);
	}

	// particle system
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].update(dt);
	}

	////////////EDITOR///////////

	update_imgui();
	
	if (m_editorTabActive == EditorTab::Library) {
		// pick position
		if (ip->mousePressed(Input::RIGHT)) {
			float3 position = m_camera.getPosition();
			float3 forward;
			if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
				// x coordinate is backwards!! Not because of mouse position but rather
				// getMousePickVector return
				float2 mpos = float2(
					1 - (float)ip->mouseX() / SCREEN_WIDTH, (float)ip->mouseY() / SCREEN_HEIGHT);
				forward = m_camera.getMousePickVector(mpos) * m_pointer_range;
			}
			else {
				forward = Normalize(m_camera.getForward()) * m_pointer_range;
			}
			// find closest collision point
			float closest = -1;
			// terrain
			float t = scene->m_terrains.castRay(position, forward);
			if (t != -1) {
				float3 point = position + forward * t;
				float diff = (point - position).Length();
				if (closest == -1 || diff < closest) {
					closest = diff;
					m_pointer = point;
				}
			}
			// entities
			for (size_t i = 0; i < scene->m_entities.size(); i++) {
				t = scene->m_entities[i]->castRay(position, Normalize(forward));
				if (t != -1) {
					float3 point = position + Normalize(forward) * t;
					float diff = (point - position).Length();
					if (closest == -1 || diff < closest) {
						closest = diff;
						m_pointer = point;
					}
				}
			}
		}
		// pick fragment
		if (ip->mousePressed(Input::MIDDLE)) {
			float3 position = m_camera.getPosition();
			float3 forward;
			if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
				// x coordinate is backwards!! Not because of mouse position but rather
				// getMousePickVector return
				float2 mpos = float2(
					1 - (float)ip->mouseX() / SCREEN_WIDTH, (float)ip->mouseY() / SCREEN_HEIGHT);
				forward = m_camera.getMousePickVector(mpos) * m_pointer_range;
			}
			else {
				forward = Normalize(m_camera.getForward()) * m_pointer_range;
			}
			Fragment* f = nullptr;
			float closest = -1;
			// terrain
			for (size_t i = 0; i < scene->m_terrains.length(); i++) {
				float t = scene->m_terrains.getTerrainFromIndex(i)->castRay(position, forward);
				if (t != -1 && (closest == -1 || t < closest)) {
					closest = t;
					f = scene->m_terrains.getTerrainFromIndex(i).get();
				}
			}
			// entity
			for (size_t i = 0; i < scene->m_entities.size(); i++) {
				float t = scene->m_entities[i]->castRay(position, forward);
				if (t != -1 && (closest == -1 || t < closest)) {
					closest = t;
					f = scene->m_entities[i].get();
				}
			}
			// animal
			for (size_t i = 0; i < scene->m_animals.size(); i++) {
				float t = scene->m_animals[i]->castRay(position, forward);
				if (t != -1 && (closest == -1 || t < closest)) {
					closest = t;
					f = scene->m_animals[i].get();
				}
			}
			// select in library
			if (f != nullptr) {
				for (size_t i = 0; i < m_library.size(); i++) {
					if (f->getID() == m_library[i]->getID()) {
						select_fragment(i);
					}
				}
			}
		}

		update_transformation(dt);
	}

	// brush
	if (m_editorTabActive == EditorTab::TerrainEditor) {
		// pick on terrain
		float3 point = m_camera.getPosition();
		float3 forward = m_camera.getForward() * 100; // MOUSE RELATIVE MODE
		if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
			float2 mpos =
				float2(1 - (float)ip->mouseX() / SCREEN_WIDTH, (float)ip->mouseY() / SCREEN_HEIGHT);
			forward = m_camera.getMousePickVector(mpos) * m_pointer_range;
		}
		float t = scene->m_terrains.castRay(point, forward);
		if (t > 0) {
			float3 intersection = point + t * forward;
			m_terrainBrush.position = intersection;
		}
		// edit mesh
		if (ip->mousePressed(m_terrainEditor_btn_raise) ||
			ip->mousePressed(m_terrainEditor_btn_lower) ||
			ip->mousePressed(m_terrainEditor_btn_flatten)) {
			scene->m_terrains.editMesh_push(); // autosave before editing
		}
		if (ip->mouseDown(m_terrainEditor_btn_raise))
			scene->m_terrains.editMesh(m_terrainBrush, Terrain::Brush::Type::Raise);
		if (ip->mouseDown(m_terrainEditor_btn_lower))
			scene->m_terrains.editMesh(m_terrainBrush, Terrain::Brush::Type::Lower);
		if (ip->mouseDown(m_terrainEditor_btn_flatten))
			scene->m_terrains.editMesh(m_terrainBrush, Terrain::Brush::Type::Flatten);
		// undo mesh
		if (ip->keyPressed(m_terrainEditor_btn_undo)) {
			scene->m_terrains.editMesh_pop();
		}
		// scroll values
		float radiusChangeOnMouseWheel = 1.1;
		if (ip->scrolledDown()) {
			if (ip->keyDown(m_terrainEditor_btn_strengthScroll))
				m_terrainBrush.strength = Clamp<float>(m_terrainBrush.strength*radiusChangeOnMouseWheel,0,1);
			else
				m_terrainBrush.radius *= radiusChangeOnMouseWheel;
		}
		else if (ip->scrolledUp()) {
			if (ip->keyDown(m_terrainEditor_btn_strengthScroll))
				m_terrainBrush.strength =
					Clamp<float>(m_terrainBrush.strength / radiusChangeOnMouseWheel, 0, 1);
			else {
				m_terrainBrush.radius /= radiusChangeOnMouseWheel;
				m_terrainBrush.radius =
					Clamp<float>(m_terrainBrush.radius, 0, m_terrainBrush.radius);
			}
		}
	}
}

void SceneEditorManager::draw_shadow() { 	// terrain manager
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<FrustumPlane> planes = shadowMap->getFrustumPlanes();

	//draw terrainBatch
	scene->m_terrains.quadtreeCull(planes);
	scene->m_terrains.draw_onlyMesh();

	// terrain entities
	vector<shared_ptr<Entity>*> culledEntities = scene->m_entities.cullElements(planes);
	for (size_t i = 0; i < culledEntities.size(); i++)
		(*culledEntities[i])->draw_onlyMesh(float3(0.));
}

void SceneEditorManager::draw_color() { 
	// Animals
	for (size_t i = 0; i < scene->m_animals.size(); ++i) {
		scene->m_animals[i]->draw();
	}

	// frustum data for culling
	vector<FrustumPlane> frustum = m_camera.getFrustumPlanes();
	// Entities
	vector<shared_ptr<Entity>*> culledEntities = scene->m_entities.cullElements(frustum);
	for (size_t i = 0; i < culledEntities.size(); i++)
		(*culledEntities[i])->draw();
	// Terrain
	scene->m_terrains.quadtreeCull(frustum);
	if (m_editorTabActive == EditorTab::TerrainEditor)
		scene->m_terrains.draw_brush(m_terrainBrush);
	else
		scene->m_terrains.draw();
	// Sea effect
	Renderer::getInstance()->copyDepthToSRV();
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i]->quadtreeCull(frustum);
		scene->m_seaEffects[i]->draw();
	}

	// SkyBox
	scene->m_skyBox.draw();

	// Dark edges
	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */

	// Particle Systems
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].draw();
	}

	//////////////////// -- EDITOR_STUFF -- ////////////////////

	if (m_editorTabActive == EditorTab::Library) {
		/* --- Things to be drawn without depthbuffer --- */
		Renderer::getInstance()->bindRenderTarget();
		draw_transformationVisuals();
		Renderer::getInstance()->bindRenderAndDepthTarget();
		/* ----------------------------------------------- */
		m_pointer_obj.setPosition(m_pointer);
		m_pointer_obj.draw_onlyMesh(float3(1, 0.5, 0.5));
	}
}

void SceneEditorManager::draw_hud() { m_crosshair.draw(); }

void SceneEditorManager::draw() { 
	setup_shadow(&m_camera); 
	draw_shadow();
	setup_color(&m_camera);
	draw_color();
	draw_hud();
}

void SceneEditorManager::load(string folder) { 
	clear();

	SceneManager::load(folder); 
	m_camera.setEye(scene->m_utility.startSpawn);

	refreshLibrary();
}

void SceneEditorManager::reset() {
	scene->reset(); 
}

void SceneEditorManager::clear() { 
	m_library.clear();
	m_transformable = nullptr;
	m_selectedIndex = -1;
	m_selectedThisFrame = false;
	scene->clear();
}
