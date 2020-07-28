#include "SceneEditorManager.h"
#include "Renderer.h"
#include "AudioHandler.h"

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
	float rotationSpeed = 0.5*0.6f/60.f;
	mouseMovement *= rotationSpeed;
	m_camera.rotate(float3(mouseMovement.x, mouseMovement.y, 0));
}

void SceneEditorManager::update_transformation(float dt) {
	Input* ip = Input::getInstance();

	// Camera properties
	float3 point = m_camera.getPosition();
	float3 direction = m_camera.getForward();

	// Pick transformation
	if (ip->mousePressed(m_key_select)) {
		vector<unique_ptr<Entity>>* entities = scene->m_repository.getEntities();
		for (size_t i = 0; i < entities->size(); i++) {
			float t = (*entities)[i]->castRay(point, direction);
			if (t != -1 && t > 0) {
				m_transformable = (*entities)[i].get();
				//(*entities)[i].get()->isVisible(false);
				break;
			}
		}
	}
	// Handle transformation
	if (m_transformable != nullptr) {
		// ImGui Show properties
		ImGui::Begin("Transformable");
		float3 t_pos = m_transformable->getPosition();
		float3 t_rot = m_transformable->getRotation();
		float3 t_scale = m_transformable->getScale();
		ImGui::Text("Position: %3.2f %3.2f %3.2f", t_pos.x, t_pos.y, t_pos.z);
		ImGui::Text("Rotation: %3.2f %3.2f %3.2f", t_rot.x, t_rot.y, t_rot.z);
		ImGui::Text("Scale   : %3.2f %3.2f %3.2f", t_scale.x, t_scale.y, t_scale.z);
		ImGui::End();
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
		}
		else {
			m_target = -1;
		}
		// Reset tranformation
		if (ip->keyPressed(m_key_resetTransform)) {
			static float3 transformStandard[Edit_Count] = { float3(0.), float3(1.), float3(0.) };
			float3 reset = transformStandard[m_transformState];
			switch (m_transformState) {
			case SceneEditorManager::Edit_Translate:
				m_transformable->setPosition(reset);
				break;
			case SceneEditorManager::Edit_Scaling:
				m_transformable->setScale(reset);
				break;
			case SceneEditorManager::Edit_Rotation:
				m_transformable->setRotation(reset);
				break;
			}
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
		if (m_transformState == Edit_Translate) {
			// arrows
			for (size_t i = 0; i < 3; i++) {
				m_arrow[i].setPosition(m_transformable->getPosition());
				m_arrow[i].setScale(m_transformable->getScale());
				m_arrow[i].draw_onlyMesh(m_axis[i]);
			}
			m_centerOrb.setPosition(m_transformable->getPosition());
			m_centerOrb.setScale(m_transformable->getScale() * 0.25f);
			m_centerOrb.draw_onlyMesh(float3(1.));
		}
		else if (m_transformState == Edit_Rotation) {
			for (size_t i = 0; i < 3; i++) {
				m_torus[i].setPosition(m_transformable->getPosition());
				m_torus[i].setRotation(m_transformable->getRotation() * m_maskPYR[i]);
				m_torus[i].setScale(m_transformable->getScale());
				m_torus[i].draw_onlyMesh(m_axis[i]);
			}
		}
		else if (m_transformState == Edit_Scaling) {
			float cubeScaling = 1;
			m_scaling_torus.setPosition(m_transformable->getPosition());
			m_scaling_torus.setScale(cubeScaling * m_transformable->getScale());
			m_scaling_torus.draw_onlyMesh(float3(1.));
		}
	}
}

void SceneEditorManager::update_imgui() {
	// Main Menu Bar
	static bool show_panel_terrains = false;
	static bool show_panel_entities = false;
	static bool show_panel_animals = false;
	static bool show_panel_seas = false;
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Empty")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Show")) {
			if (ImGui::MenuItem("terrains"))
				show_panel_terrains = !show_panel_terrains;
			if (ImGui::MenuItem("entities"))
				show_panel_entities = !show_panel_entities;
			if (ImGui::MenuItem("animals"))
				show_panel_animals = !show_panel_animals;
			if (ImGui::MenuItem("seas"))
				show_panel_seas = !show_panel_seas;
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	if (show_panel_terrains && ImGui::Begin("Terrains")) {
		static int tInd = 0;
		static float3 position, rotation, scale;
		static AreaTag tag = AreaTag::Forest;
		static string heightmap = m_heightmap_textures[0]->filename;
		static string textures[4] = { m_terrain_textures[0]->filename,
			m_terrain_textures[0]->filename, m_terrain_textures[7]->filename,
			m_terrain_textures[7]->filename };
		static XMINT2 subSize = XMINT2(15,15);
		static XMINT2 divisions = XMINT2(16,16);
		static float3 wind;
		ImGui::InputInt("Terrain index", &tInd);
		ImGui::InputFloat3("Position", (float*)&position, 2);
		ImGui::InputFloat3("Rotation", (float*)&rotation, 2);
		ImGui::InputFloat3("Scale", (float*)&scale, 2);
		ImGui::InputInt2("SubSize", (int*)&subSize);
		ImGui::InputInt2("Divisions", (int*)&divisions);
		ImGui::InputFloat3("Wind", (float*)&wind, 1);
		if (ImGui::BeginCombo("AreaTag", AreaTagToString(tag).c_str())) {
			for (size_t i = 0; i < AreaTag::NR_OF_AREAS; i++) {
				if (ImGui::MenuItem(AreaTagToString((AreaTag)i).c_str()))
					tag = (AreaTag)i;
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Heightmap", heightmap.c_str())) {
			float cWidth = ImGui::CalcItemWidth();
			int itemCountOnWidth = 3;
			for (size_t i = 0; i < m_heightmap_textures.size(); i++) {
				if (ImGui::ImageButton(m_heightmap_textures[i]->view.Get(),
						ImVec2(cWidth / itemCountOnWidth, cWidth / itemCountOnWidth)))
					heightmap = m_heightmap_textures[i]->filename;
				if ((i + 1) % itemCountOnWidth != 0)
					ImGui::SameLine();
			}
			ImGui::EndCombo();
		}
		string tex_description[4] = { "Texture Flat","Texture Bottom Flat", "Texture Steep", "Texture Minimal Steep" };
		for (size_t i = 0; i < 4; i++) {
			if (ImGui::BeginCombo(tex_description[i].c_str(), textures[i].c_str())) {
				float cWidth = ImGui::CalcItemWidth();
				int itemCountOnWidth = 3;
				for (size_t j = 0; j < m_terrain_textures.size(); j++) {
					if (ImGui::ImageButton(m_terrain_textures[j]->view.Get(),
							ImVec2(cWidth / itemCountOnWidth, cWidth / itemCountOnWidth)))
						textures[i] = m_terrain_textures[j]->filename;
					if ((j + 1) % itemCountOnWidth != 0)
						ImGui::SameLine();
				}
				ImGui::EndCombo();
			}
		}
		if (ImGui::Button("Create")) {
			scene->m_terrains.add(
				position, scale, heightmap, textures, subSize, divisions, wind, tag);
		}
		ImGui::SameLine();
		if (ImGui::Button("Apply")) {
			shared_ptr<Terrain> terrain = scene->m_terrains.getTerrainFromIndex(tInd);
			terrain->setPosition(position);
			terrain->setRotation(rotation);
			terrain->setScale(scale);
			terrain->setTextures(textures);
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			scene->m_terrains.remove(tInd);
		}

		string collapseHeader = "All terrains (" + to_string(scene->m_terrains.length()) + ")";
		if (ImGui::CollapsingHeader(collapseHeader.c_str())) {
			for (size_t i = 0; i < scene->m_terrains.length(); i++) {
				shared_ptr<Terrain> terrain = scene->m_terrains.getTerrainFromIndex(i);
				if (ImGui::BeginChild(("Terrain " + to_string(i)).c_str(), ImVec2(0, 100), true)) {
					float3 t_pos = terrain->getPosition();
					float3 t_rot = terrain->getRotation();
					float3 t_scale = terrain->getScale();
					ImGui::Text("Position: %.2f %.2f %.2f", t_pos.x, t_pos.y, t_pos.z);
					ImGui::Text("Rotation: %.2f %.2f %.2f", t_rot.x, t_rot.y, t_rot.z);
					ImGui::Text("Scale   : %.2f %.2f %.2f", t_scale.x, t_scale.y, t_scale.z);
					ImGui::Text(("AreaTag: " + AreaTagToString(terrain->getTag())).c_str());
					ImGui::EndChild();
				}
			}
		}
		ImGui::End();
	}
	if (show_panel_entities && ImGui::Begin("Entities")) {
		
	}
	if (show_panel_animals && ImGui::Begin("Animals")) {
		static string models[3] = {"Bear","Goat","Gorilla"};
		static int pickedAnimal = -1;
		static float3 position, rotation, scale, sleepPosition;
		static string model = models[0];
		static float range_player = 0, range_fruit = 0;
		static FruitType fruitType = FruitType::APPLE;
		static int fruitCount = 1;
		static float throwStrength = 0;
		if (ImGui::BeginCombo("Animals",pickedAnimal==-1?"None":to_string(pickedAnimal).c_str())) {
			for (size_t i = 0; i < scene->m_animals.size(); i++) {
				if (ImGui::MenuItem(to_string(i).c_str())) {
					pickedAnimal = i;
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("UpdateP"))
			position = m_pointer;
		ImGui::SameLine();
		ImGui::InputFloat3("Position", (float*)&position, 2);
		ImGui::InputFloat3("Rotation", (float*)&rotation, 2);
		ImGui::InputFloat3("Scale", (float*)&scale, 2);
		if (ImGui::Button("UpdateSP"))
			sleepPosition = m_pointer;
		ImGui::SameLine();
		ImGui::InputFloat3("Sleep Position", (float*)&sleepPosition, 2);
		if (ImGui::BeginCombo("Model", model.c_str())) {
			for (size_t i = 0; i < 3; i++) {
				if (ImGui::MenuItem(models[i].c_str()))
					model = models[i];
			}
			ImGui::EndCombo();
		}
		ImGui::InputFloat("Range To Player", &range_player);
		ImGui::InputFloat("Range To Fruits", &range_fruit);
		if (ImGui::BeginCombo("Fruit To Give", FruitTypeToString(fruitType).c_str())) {
			for (size_t i = 0; i < FruitType::NR_OF_FRUITS; i++) {
				if (ImGui::MenuItem(FruitTypeToString((FruitType)i).c_str()))
					fruitType = (FruitType)i;
			}
			ImGui::EndCombo();
		}
		ImGui::InputInt("Fruit Count", &fruitCount);
		ImGui::InputFloat("Throw Strength", &throwStrength);
		if (ImGui::Button("Create")) {
			scene->m_animals.push_back(Animal(model, range_player, range_fruit, (int)fruitType, fruitCount, throwStrength, position, sleepPosition, rotation.y));
		}
		ImGui::SameLine();
		if (ImGui::Button("Apply")) {
			if (pickedAnimal >= 0 && pickedAnimal < scene->m_animals.size()) {
				scene->m_animals[pickedAnimal].setPosition(position);
				scene->m_animals[pickedAnimal].setRotation(rotation);
				scene->m_animals[pickedAnimal].setScale(scale);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Remove")) {
			if (pickedAnimal >= 0 && pickedAnimal < scene->m_animals.size()) {
				scene->m_animals.erase(scene->m_animals.begin() + pickedAnimal);
				pickedAnimal = -1;
			}
		}
	}
	if (show_panel_seas && ImGui::Begin("seas")) {
	
	}
}
SceneEditorManager::SceneEditorManager() { 
	m_animationTest.load("apple_smooth");

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

}

void SceneEditorManager::update() {
	Input* ip = Input::getInstance();

	scene->m_timer.update();
	float dt = scene->m_timer.getDt();

	// update camera
	updateCameraMovement(dt);

	// Update Skybox
	scene->m_skyBox.update(dt);
	const Terrain* activeTerrain = scene->m_terrains.getTerrainFromPosition(m_camera.getPosition());
	if (activeTerrain != nullptr) {
		AreaTag tag = activeTerrain->getTag();
		scene->m_skyBox.switchLight(tag);
		if (!m_manualCamera)
			AudioHandler::getInstance()->changeMusicByTag(tag, dt);
	}

	// update water
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i].update(dt);
	}

	// particle system
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i].update(dt);
	}

	////////////EDITOR///////////
	
	if (ip->mousePressed(Input::RIGHT)) {
		float3 position = m_camera.getPosition();
		float3 forward;
		if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
			// x coordinate is backwards!! Not because of mouse position but rather getMousePickVector return
			float2 mpos = float2(1 - (float)ip->mouseX() / SCREEN_WIDTH, (float)ip->mouseY() / SCREEN_HEIGHT);
			forward = m_camera.getMousePickVector(mpos) * m_pointer_range;
		}
		else {
			forward = Normalize(m_camera.getForward()) * m_pointer_range;
		}
		float t = scene->m_terrains.castRay(position, forward);
		if (t > 0) {
			m_pointer = position + forward * t;
			m_anim_position = m_pointer + float3(0, m_anim_heightAboveGround+0.1, 0);
			m_anim_velocity = float3(0.);
		}
	}

	if (ImGui::Begin("Animation Test")) {
		ImGui::Text(
			"Position: %.1f %.1f %.1f", m_anim_position.x, m_anim_position.y, m_anim_position.z);
		ImGui::Text(
			"Velocity: %.1f %.1f %.1f", m_anim_velocity.x, m_anim_velocity.y, m_anim_velocity.z);
		string state = (m_anim_state == AnimationState::bouncing ? "Bounce" : "OnGround");
		ImGui::Text(("State: "+state).c_str());
		ImGui::Text("bounce pos: %.1f", m_anim_bounce_pos);
		ImGui::Text("bounce vel: %.1f", m_anim_bounce_vel);
		ImGui::InputFloat("Height above ground", &m_anim_heightAboveGround);
		ImGui::InputFloat("Gravity Strength", &m_anim_gravity_strength);
		ImGui::InputFloat("Jump Strength", &m_anim_jump_strength);
		ImGui::InputFloat("Collision slowdown", &m_anim_groundHitSlowdown);
		ImGui::InputFloat("Bounce slowdown", &m_anim_bounce_slowdown);
		ImGui::InputFloat("Bounce on jump effect strength", &m_anim_bounce_onJumpEffect);
		ImGui::InputFloat("Bounce on hit effect strength", &m_anim_bounce_onHitEffect);
		ImGui::InputFloat("Bounce visual speed", &m_anim_bounce_speed);
		ImGui::InputFloat("LookAt Speed", &m_anim_lookat_speed);
		ImGui::End();
	}

	// test animation
	if (m_anim_state == bouncing) {
		// bounce physics
		float3 movement = m_anim_velocity * dt;
		float t = getScene()->m_terrains.castRay(
			m_anim_position + float3(0, -m_anim_heightAboveGround, 0), movement);
		if (t >= 0) {
			float3 collision =
				m_anim_position + float3(0, -m_anim_heightAboveGround, 0) + movement * t;
			float3 collision_normal = getScene()->m_terrains.getNormalFromPosition(collision);
			if (rand() % 2 == 0 && collision_normal.Dot(float3(0, 1, 0)) > 0.5) {
				m_anim_bounce_vel = m_anim_bounce_onJumpEffect;
				float deg = RandomFloat() * 2 * 3.1415;
				m_anim_velocity = Normalize(float3(cos(deg), 1, sin(deg))) * m_anim_jump_strength;
				m_anim_state = bouncing;
			}
			else {
				float3 normVel = Normalize(m_anim_velocity);
				// bounce
				m_anim_bounce_vel += -m_anim_velocity.Length() * m_anim_bounce_onHitEffect *
									 normVel.Dot(collision_normal);

				float3 reflected = -m_anim_velocity + 2 * (-m_anim_velocity.Dot(collision_normal) * collision_normal -
										  -m_anim_velocity);
				m_anim_velocity = reflected;

				//m_anim_velocity.y *= -1;
				m_anim_velocity *= m_anim_groundHitSlowdown;
			}
		}

		//update movement
		m_anim_position += m_anim_velocity * dt;
		m_anim_velocity += m_anim_gravity_dir * m_anim_gravity_strength * dt;
		//update bounce visual
		m_anim_bounce_pos += m_anim_bounce_vel * dt;
		m_anim_bounce_vel += (m_anim_bounce_des - m_anim_bounce_pos) * m_anim_bounce_speed * dt;
		m_anim_bounce_vel *= pow(m_anim_bounce_slowdown, dt);

		float3 desPos = Normalize(m_anim_velocity * float3(1, 0, 1));
		m_lookPosition += (desPos - m_lookPosition) * m_anim_lookat_speed * dt;

		if (m_anim_velocity.Length() < 0.1) {
			//m_anim_state = onGround;
		}
	}
	else if (m_anim_state == onGround) {
		//m_anim_bounce_vel = m_anim_bounce_onJumpEffect;
		//m_anim_velocity = Normalize(float3(1, 1, 0)) * m_anim_jump_strength;
		//m_anim_state = bouncing;
		m_anim_state = bouncing;
	}

	update_transformation(dt);

	update_imgui();

}

void SceneEditorManager::draw_shadow() { 	// terrain manager
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<FrustumPlane> planes = shadowMap->getFrustumPlanes();

	//draw terrainBatch
	scene->m_terrains.quadtreeCull(planes);
	scene->m_terrains.draw_onlyMesh();

	// terrain entities
	scene->m_repository.quadtreeCull(planes);
	scene->m_repository.draw_onlyMesh();

	m_animationTest.draw_onlyMesh(float3(0.));
}

void SceneEditorManager::draw_color() { 
	m_animationTest.setPosition(m_anim_position);
	float3 scale = float3(1.) * 0.35;
	scale.y *= m_anim_bounce_pos;
	scale.x *= 1.f/m_anim_bounce_pos;
	scale.z *= 1.f/m_anim_bounce_pos;
	m_animationTest.setScale(scale);
	if (m_lookPosition.Length() > 0.001)
		m_animationTest.lookTo(m_lookPosition);
	m_animationTest.draw();

	// Animals
	for (size_t i = 0; i < scene->m_animals.size(); ++i) {
		scene->m_animals[i].draw();
	}

	// frustum data for culling
	vector<FrustumPlane> frustum = m_camera.getFrustumPlanes();
	// Entities
	scene->m_repository.quadtreeCull(frustum);
	scene->m_repository.draw();
	// Terrain
	scene->m_terrains.quadtreeCull(frustum);
	scene->m_terrains.draw();
	// Sea effect
	Renderer::getInstance()->copyDepthToSRV();
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i].quadtreeCull(frustum);
		scene->m_seaEffects[i].draw();
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

	/* --- Things to be drawn without depthbuffer --- */
	Renderer::getInstance()->bindRenderTarget();
	draw_transformationVisuals();
	Renderer::getInstance()->bindRenderAndDepthTarget();
	/* ----------------------------------------------- */
	m_pointer_obj.setPosition(m_pointer);
	m_pointer_obj.draw_onlyMesh(float3(1,0.5,0.5));
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
	SceneManager::load(folder); 
	m_camera.setEye(scene->m_playerStartPos);
}
