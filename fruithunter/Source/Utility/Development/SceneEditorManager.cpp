#include "SceneEditorManager.h"
#include "Renderer.h"
#include "AudioController.h"
#include "PathFindingThread.h"
#include "Settings.h"
#include <WICTextureLoader.h>
#include <ScreenGrab.h>
#include <wincodec.h>
#include <fstream>

void SceneEditorManager::update_imgui_leaderboard() {
	static bool initDownload = false;
	ImGui::InputText("Steam Leaderboard Name", &scene->m_leaderboardName);
	if (ImGui::Button("Update")) {
		m_leaderboard.FindLeaderboard(scene->m_leaderboardName.c_str());
		initDownload = false;
	}
	ImGui::SameLine();
	static ELeaderboardDataRequest requestType =
		ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobalAroundUser;
	const string requestTypesStr[] = { "Global", "GlobalAroundUser", "Friends", "Users" };
	if (ImGui::BeginCombo("RequestType", requestTypesStr[requestType].c_str())) {
		for (size_t i = 0; i < 4; i++) {
			if (ImGui::MenuItem(requestTypesStr[i].c_str())) {
				requestType = (ELeaderboardDataRequest)i;
				// fetch again
				if (m_leaderboard.getRequestState_FindLeaderboard() ==
					CSteamLeaderboard::RequestState::r_inactive)
					m_leaderboard.FindLeaderboard(scene->m_leaderboardName.c_str());
				initDownload = false;
			}
		}
		ImGui::EndCombo();
	}
	if (m_leaderboard.getRequestState_FindLeaderboard() ==
			CSteamLeaderboard::RequestState::r_finished &&
		!initDownload) {
		int begin = -4, end = 5;
		if (requestType == ELeaderboardDataRequest::k_ELeaderboardDataRequestGlobal)
			begin = 0, end = 10;
		m_leaderboard.DownloadScores(requestType, begin, end);
		initDownload = true;
	}
	string status = "";
	string status_strOptions[4] = { "Idle", "Failed", "Waiting", "Finished" };
	CSteamLeaderboard::RequestState reqState = m_leaderboard.getRequestState_DownloadScore();
	ImGui::Text("Request Status: %s", status_strOptions[reqState].c_str());
	if (ImGui::ListBoxHeader("Player Score", 10)) {
		for (size_t i = 0; i < m_leaderboard.getEntryCount(); i++) {
			LeaderboardEntry_t entry;
			if (m_leaderboard.getEntry(i, entry)) {
				ImVec4 color = ImVec4(1, 1, 1, 1);
				const char* myname = SteamFriends()->GetPersonaName();
				const char* name = SteamFriends()->GetFriendPersonaName(entry.m_steamIDUser);
				if (myname != nullptr && name != nullptr && string(myname) == string(name))
					color = ImVec4(1, 1, 0, 1);
				ImGui::TextColored(
					ImVec4(1, 1, 1, 1), "%s[%i]: %i", name, entry.m_nGlobalRank, entry.m_nScore);
			}
		}
		ImGui::ListBoxFooter();
	}
	static int score = 0;
	ImGui::InputInt("Score", &score);
	ImGui::SameLine();
	if (ImGui::Button("Upload")) {
		m_leaderboard.UploadScore(score,
			ELeaderboardUploadScoreMethod::k_ELeaderboardUploadScoreMethodForceUpdate); // overwrite
	}
}

void SceneEditorManager::update_imgui_library() {
	static const string libraryTabsStr[LibraryTab::tab_count] = { "Terrain", "Entity", "Sea",
		"Particle System", "Effect" , "World Message"};
	const ImVec4 colorBlueActive = ImVec4(51 / 255.f, 105 / 255.f, 173 / 255.f, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
	for (size_t i = 0; i < tab_count; i++) {
		bool pop = false;
		if (m_libraryTabOpen == (LibraryTab)i) {
			ImGui::PushStyleColor(ImGuiCol_Button, colorBlueActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorBlueActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorBlueActive);
			pop = true;
		}
		if (ImGui::Button(libraryTabsStr[i].c_str())) {
			m_libraryTabOpen = (LibraryTab)i;
		}
		if (pop)
			ImGui::PopStyleColor(3);
		if (i + 1 < tab_count)
			ImGui::SameLine();
	}
	ImGui::PopStyleVar(1);
	ImGui::PushStyleColor(ImGuiCol_Separator, colorBlueActive);
	ImGui::Separator();
	ImGui::PopStyleColor(1);

	ImGui::BeginGroup();
	switch (m_libraryTabOpen) {
	case SceneEditorManager::tab_terrain:
		update_panel(scene->m_terrains, m_libraryTabOpen,
			&SceneEditorManager::update_panel_terrain_unselected);
		break;
	case SceneEditorManager::tab_entity:
		update_panel_entity_improved(scene->m_entities, m_libraryTabOpen,
			&SceneEditorManager::update_panel_entity_unselected);
		break;
	case SceneEditorManager::tab_sea:
		update_panel(scene->m_seaEffects, m_libraryTabOpen,
			&SceneEditorManager::update_panel_sea_unselected);
		break;
	case SceneEditorManager::tab_particleSystem:
		update_panel(scene->m_particleSystems, m_libraryTabOpen,
			&SceneEditorManager::update_panel_ps_unselected);
		break;
	case SceneEditorManager::tab_effect:
		update_panel(scene->m_effects, m_libraryTabOpen,
			&SceneEditorManager::update_panel_effect_unselected);
		break;
	case SceneEditorManager::tab_worldMessage:
		update_panel(scene->m_worldMessages, m_libraryTabOpen,
			&SceneEditorManager::update_panel_worldMessage_unselected);
	}
	ImGui::EndGroup();
	select_index(m_libraryTabOpen, m_library_selections[m_libraryTabOpen]);
}

void SceneEditorManager::update_imgui_gameRules() {
	ImGui::SetNextItemWidth(100);
	ImGui::InputText("Scene Name", &scene->m_sceneName);
	for (size_t i = 0; i < NR_OF_FRUITS; i++) {
		ImGui::SetNextItemWidth(100);
		ImGui::InputInt(("WinCondition (" + FruitTypeToString((FruitType)i) + ")").c_str(),
			&scene->m_utility.winCondition[i]);
	}
	for (size_t i = 0; i < NR_OF_TIME_TARGETS; i++) {
		ImGui::Text(Milliseconds2DisplayableString(scene->m_utility.timeTargets[i]).c_str());
		ImGui::SameLine();
		ImGui::SetNextItemWidth(100);
		ImGui::InputInt(("Time Target (" + TimeTargetToString((TimeTargets)i) + ")").c_str(),
			(int*)&scene->m_utility.timeTargets[i]);
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
	static float falloffPoints[25] = { -1 };
	ImGui::SetNextItemWidth(200);
	if (ImGui::SliderFloat("Falloff", &m_terrainBrush.falloff, 0, 5) || falloffPoints[0] == -1) {
		for (size_t i = 0; i < ARRAYSIZE(falloffPoints); i++) {
			float x = (float)i / (ARRAYSIZE(falloffPoints) - 1);
			falloffPoints[i] =
				1 - pow(1 - 0.5 * (1 - cos(x * 3.1415f)), 1.f / m_terrainBrush.falloff);
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

void SceneEditorManager::update_panel_entity_improved(
	QuadTree<shared_ptr<Entity>>& list, LibraryTab tab, void (SceneEditorManager::*func)(void)) {
	int& selectedIdx = m_library_selections[tab];
	Input* ip = Input::getInstance();
	if (ImGui::BeginChild(
			"FragmentDisplayer", ImVec2(200, 0), true, ImGuiWindowFlags_AlwaysAutoResize)) {
		// List
		ImVec2 panelSize = ImGui::GetWindowSize();
		if (ImGui::BeginChildFrame(2, ImVec2(0, panelSize.y - 135))) {
			for (size_t i = 0; i < list.size(); i++) {
				ImGui::PushID(i);
				bool selected = (selectedIdx == i);
				if (ImGui::Selectable(
						static_cast<Fragment*>(list[i].get())->getFullDescription().c_str(),
						&selected)) {
					if (selectedIdx == i)
						selectedIdx = -1;
					else
						selectedIdx = i;
				}
				//// Drag Source
				// if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
				//	ImGui::SetDragDropPayload("DND_TerrainItem", &i, sizeof(size_t));

				//	ImGui::Text("Dragging[%i]", i);
				//	ImGui::EndDragDropSource();
				//}
				//// Drag Target
				// if (ImGui::BeginDragDropTarget()) {
				//	if (const ImGuiPayload* payload =
				//			ImGui::AcceptDragDropPayload("DND_TerrainItem")) {
				//		size_t payload_i = *(size_t*)payload->Data;
				//		shared_ptr<Entity> payload_source = list[payload_i];
				//		list.erase(list.begin() + payload_i);
				//		list.insert(list.begin() + i, payload_source);
				//	}
				//	ImGui::EndDragDropTarget();
				//}
				ImGui::PopID();
			}
			ImGui::EndChildFrame();
		}
		// buttons
		if ((ImGui::Button("Remove") || ip->keyPressed(Keyboard::Delete)) && selectedIdx != -1) {
			list.remove(selectedIdx);
			if (list.size() == 0)
				selectedIdx = -1;
			else
				selectedIdx = Clamp(selectedIdx, 0, (int)list.size() - 1);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("QuickButton (Del)");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if ((ImGui::Button("Duplicate") || ip->keyPressed(Keyboard::C)) && selectedIdx != -1) {
			shared_ptr<Entity> duplicate = make_shared<Entity>(*list[selectedIdx].get());
			list.add(duplicate->getLocalBoundingBoxPosition(), duplicate->getLocalBoundingBoxSize(),
				duplicate);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("QuickButton (C)");
			ImGui::EndTooltip();
		}
		if ((ImGui::Button("Find") || ip->keyPressed(Keyboard::F)) && selectedIdx != -1) {
			Transformation* t = dynamic_cast<Transformation*>(list[selectedIdx].get());
			if (t != nullptr) {
				scene->m_camera.setEye(t->getPosition() + float3(1, 1, 0) * 2.f);
				scene->m_camera.setTarget(t->getPosition());
			}
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("QuickButton (F)");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if ((ImGui::Button("Move to Pointer") || ip->keyPressed(Keyboard::V)) &&
			selectedIdx != -1) {
			Transformation* t = dynamic_cast<Transformation*>(list[selectedIdx].get());
			if (t != nullptr) {
				t->setPosition(m_pointer);
			}
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("QuickButton (V)");
			ImGui::EndTooltip();
		}
		// help text
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 3));
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
	}
	ImGui::EndChild();
	ImGui::SameLine();
	ImGui::BeginGroup();
	if (selectedIdx != -1) {
		static_cast<Fragment*>(list[selectedIdx].get())->imgui_properties();
	}
	else {
		std::invoke(func, *this);
	}
	ImGui::EndGroup();
}

void SceneEditorManager::updateCameraMovement(float dt) {
	Input* ip = Input::getInstance();
	// Position
	const float3 STD_Forward = float3(0.0f, 0.0f, 1.0f);
	float3 forward = scene->m_camera.getForward();
	float3 up = scene->m_camera.getUp();
	float3 right = scene->m_camera.getRight();
	float3 acceleration = forward * (float)(ip->keyDown(KEY_FORWARD) - ip->keyDown(KEY_BACKWARD)) +
						  right * (float)(ip->keyDown(KEY_RIGHT) - ip->keyDown(KEY_LEFT)) +
						  up * (float)(ip->keyDown(KEY_UP) - ip->keyDown(KEY_DOWN));
	acceleration.Normalize();
	float speed = ip->keyDown(KEY_SLOW) ? m_lowSpeed : m_highSpeed;
	acceleration *= speed;

	m_cam_velocity += acceleration * dt;
	scene->m_camera.move(m_cam_velocity * dt);
	m_cam_velocity *= pow(m_cam_friction / 60.f, dt); // friction/slowdown
	// Rotation
	float2 mouseMovement;
	if (ip->getMouseMode() == DirectX::Mouse::MODE_RELATIVE) {
		mouseMovement = float2((float)ip->getMouseMovement().x, (float)ip->getMouseMovement().y);
	}
	float rotationSpeed = Settings::getInstance()->getSensitivity() * 0.01f;
	mouseMovement *= rotationSpeed;
	scene->m_camera.rotate(float3(mouseMovement.y, mouseMovement.x, 0));
}

void SceneEditorManager::update_transformation(float dt) {
	Input* ip = Input::getInstance();

	// Handle transformation
	if (m_transformable != nullptr) {
		// Camera transformation (+mouse when released)
		float3 cam_position = scene->m_camera.getPosition();
		float3 cam_forward = scene->m_camera.getForward();
		if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
			// x coordinate is backwards!! Not because of mouse position but rather
			// getMousePickVector return
			float2 mpos = ip->mouseXY() / float2(1280, 720);
			mpos.x = 1 - mpos.x;
			cam_forward = scene->m_camera.getMousePickVector(mpos);
		}

		// target transform option
		if (ip->mousePressed(m_key_target)) {
			// ray cast
			if (m_transformState == Edit_Translate) {
				float t_min = -1;
				for (size_t i = 0; i < 3; i++) {
					float t = m_arrow[i].castRay(cam_position, cam_forward);
					if (t != -1 && (t_min == -1 || t < t_min)) {
						t_min = t;
						m_target = i;
					}
				}
				float t = m_centerOrb.castRay(cam_position, cam_forward);
				if (t != -1 && (t_min == -1 || t < t_min)) {
					t_min = t;
					m_target = 4; // center orb
					m_target_pos = scene->m_camera.getPosition();
				}
				m_target_pos = cam_position;
				m_target_forward = cam_forward;
				m_target_rayDist = t_min;
			}
			else if (m_transformState == Edit_Rotation) {
				float t_min = -1;
				for (size_t i = 0; i < 3; i++) {
					float t = m_torus[i].castRay(cam_position, cam_forward);
					if (t != -1 && (t_min == -1 || t < t_min)) {
						t_min = t;
						m_target = i;
					}
				}
				m_target_pos = cam_position;
				m_target_forward = cam_forward;
				m_target_rayDist = t_min;
			}
			else if (m_transformState == Edit_Scaling) {
				float t = m_scaling_torus.castRay(cam_position, cam_forward);
				if (t != -1) {
					m_target = 4; // center orb
					m_target_pos = cam_position;
				}
				m_target_pos = cam_position;
				m_target_forward = cam_forward;
				m_target_rayDist = t;
			}
		}
		else if (ip->mouseDown(m_key_target) && m_target != -1) {
			float3 posDiff = cam_position - m_target_pos;
			float3 forwardDiff = cam_forward - m_target_forward;
			float3 target_forward = m_target_forward;
			float3 target_pos = m_target_pos;
			m_target_pos = cam_position;
			m_target_forward = cam_forward;
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
				float3 point = cam_position + cam_forward * m_target_rayDist;
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
		Renderer::getInstance()->setBlendState_NonPremultiplied();
		float alpha = 0.65f;
		float scale1 = m_transformable->getScale().Length();
		scale1 = Clamp<float>(scale1, 1, 10);
		float3 scale(scale1, scale1, scale1);
		if (m_transformState == Edit_Translate) {
			// arrows
			for (size_t i = 0; i < 3; i++) {
				m_arrow[i].setPosition(m_transformable->getPosition());
				m_arrow[i].setScale(scale);
				m_arrow[i].draw_onlyMesh(m_axis[i], alpha);
			}
			m_centerOrb.setPosition(m_transformable->getPosition());
			m_centerOrb.setScale(scale * 0.25f);
			m_centerOrb.draw_onlyMesh(float3(1.f), alpha);
		}
		else if (m_transformState == Edit_Rotation) {
			for (size_t i = 0; i < 3; i++) {
				m_torus[i].setPosition(m_transformable->getPosition());
				m_torus[i].setRotation(m_transformable->getRotation() * m_maskPYR[i]);
				m_torus[i].setScale(scale);
				m_torus[i].draw_onlyMesh(m_axis[i], alpha);
			}
		}
		else if (m_transformState == Edit_Scaling) {
			float cubeScaling = 1;
			m_scaling_torus.setPosition(m_transformable->getPosition());
			m_scaling_torus.setScale(cubeScaling * scale);
			m_scaling_torus.draw_onlyMesh(float3(1.f), alpha);
		}
		Renderer::getInstance()->setBlendState_Opaque();
	}
}

void SceneEditorManager::update_imgui() {
	Input* ip = Input::getInstance();
	ImVec2 screenSize = ImVec2(
		Renderer::getInstance()->getScreenWidth(), Renderer::getInstance()->getScreenHeight());

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
			if (ImGui::MenuItem("Load and Save Scenes")) {
				ErrorLogger::log(" ----- Begin Reevaluating Scenes -----");
				for (size_t i = 0; i < m_loadable_scenes.size(); i++) {
					load(m_loadable_scenes[i]);
					scene->save();
				}
				ErrorLogger::log("Done Reevaluating Scenes");
			}
			ImGui::EndMenu();
		}
		menuBarSize = ImGui::GetWindowSize();
		ImGui::EndMainMenuBar();
	}
	// Docked window
	float windHeight = screenSize.y - menuBarSize.y;
	ImGui::SetNextWindowPos(ImVec2(0, menuBarSize.y));
	ImGui::SetNextWindowSizeConstraints(
		ImVec2(0, windHeight), ImVec2(Renderer::getInstance()->getScreenWidth(), windHeight));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (ImGui::Begin("Docked Window", NULL,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize)) {
		static const string editorTabsStr[EditorTab::etab_count] = { "Library", "Game Rules",
			"Terrain Editor", "Leaderboard" };
		const ImVec4 colorBlueActive = ImVec4(51 / 255.f, 105 / 255.f, 173 / 255.f, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 0));
		for (size_t i = 0; i < etab_count; i++) {
			bool pop = false;
			if (m_editorTabActive == (EditorTab)i) {
				ImGui::PushStyleColor(ImGuiCol_Button, colorBlueActive);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorBlueActive);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorBlueActive);
				pop = true;
			}
			if (ImGui::Button(editorTabsStr[i].c_str())) {
				m_editorTabActive = (EditorTab)i;
			}
			if (pop)
				ImGui::PopStyleColor(3);
			if (i + 1 < etab_count)
				ImGui::SameLine();
		}
		ImGui::PopStyleVar(1);
		ImGui::PushStyleColor(ImGuiCol_Separator, colorBlueActive);
		ImGui::Separator();
		ImGui::PopStyleColor(1);
		ImGui::BeginGroup();
		switch (m_editorTabActive) {
		case SceneEditorManager::Library:
			update_imgui_library();
			break;
		case SceneEditorManager::GameRules:
			update_imgui_gameRules();
			break;
		case SceneEditorManager::TerrainEditor:
			update_imgui_terrainEditor();
			break;
		case SceneEditorManager::Leaderboard:
			update_imgui_leaderboard();
			break;
		}
		ImGui::EndGroup();
	}
	ImGui::End();
	ImGui::PopStyleVar();

	// bottom right docked window
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	if (ImGui::Begin("win_camProperties", NULL,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoMove)) {
		float3 cp = scene->m_camera.getPosition();
		float3 cn = scene->m_camera.getForward();
		ImGui::Text("Cam Pos: [%.2f %.2f %.2f] Cam Dir: [%.2f %.2f %.2f]", cp.x, cp.y, cp.z, cn.x,
			cn.y, cn.z);
		ImVec2 size = ImGui::GetWindowSize();
		ImGui::SetWindowPos(screenSize - size);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void SceneEditorManager::select_index(LibraryTab tab, int index) {
	m_libraryTabOpen = tab;
	m_library_selections[tab] = index;

	m_transformable = nullptr;
	if (index != -1) {
		switch (tab) {
		case SceneEditorManager::tab_terrain:
			m_transformable = static_cast<Transformation*>(scene->m_terrains[index].get());
			break;
		case SceneEditorManager::tab_entity:
			m_transformable = static_cast<Transformation*>(scene->m_entities[index].get());
			break;
		case SceneEditorManager::tab_sea:
			m_transformable = static_cast<Transformation*>(scene->m_seaEffects[index].get());
			break;
		case SceneEditorManager::tab_particleSystem:
			m_transformable = static_cast<Transformation*>(scene->m_particleSystems[index].get());
			break;
		case SceneEditorManager::tab_effect:
			m_transformable = static_cast<Transformation*>(scene->m_effects[index].get());
			break;
		case SceneEditorManager::tab_worldMessage:
			m_transformable = static_cast<Transformation*>(scene->m_worldMessages[index].get());
			break;
		}
	}
}

void SceneEditorManager::select_fragment(FragmentID id) {
	// terrain
	for (size_t i = 0; i < scene->m_terrains.size(); i++) {
		if (scene->m_terrains[i]->getID() == id) {
			select_index(LibraryTab::tab_terrain, i);
			return;
		}
	}
	// entity
	for (size_t i = 0; i < scene->m_entities.size(); i++) {
		if (scene->m_entities[i]->getID() == id) {
			select_index(LibraryTab::tab_entity, i);
			return;
		}
	}
}

void SceneEditorManager::readSceneDirectory() {
	SimpleFilesystem::readDirectory("assets/Scenes", m_loadable_scenes);
}

void SceneEditorManager::update_panel_entity_unselected() {
	Input* ip = Input::getInstance();
	// _Properties_
	// - rotation
	if (ImGui::Checkbox("Random Rotation", &m_entityGhost_randomRotation) &&
		m_entityGhost_randomRotation)
		entityGhost_randomizeProperties();
	if (!m_entityGhost_randomRotation)
		ImGui::SliderFloat("Rotation Y", &m_entityGhost_rotationY, 0, 2 * XM_PI, "%.2f rad");
	// - scale
	if (ImGui::Checkbox("Random Scale", &m_entityGhost_randomScale) && m_entityGhost_randomScale)
		entityGhost_randomizeProperties();
	if (m_entityGhost_randomScale) {
		if (ImGui::DragFloatRange2("Scale Range", &m_entityGhost_scaleRange.x,
				&m_entityGhost_scaleRange.y, 0.01f, 0, 10, "Min: %.2f", "Max: %.2f"))
			entityGhost_randomizeProperties();
	}
	else
		ImGui::SliderFloat("Scale", &m_entityGhost_scale, 0, 10, "%.2f");
	// collision
	bool collidable = m_entityGhost_placer.getIsCollidable();
	if (ImGui::Checkbox("Collidable", &collidable))
		m_entityGhost_placer.setCollidable(collidable);
	// - quick commands
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 3));
	ImGui::Text(" --- Quick Commands ---");
	ImVec4 btnCol(0, 1, 0, 1);
	ImGui::Text("Rotate: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "Scroll Up/Down");
	ImGui::Text("Scaling: ");
	ImGui::SameLine();
	ImGui::TextColored(btnCol, "Left Shift + Scroll Up/Down");
	ImGui::PopStyleVar();
	if (ip->keyDown(Keyboard::LeftShift))
		m_entityGhost_scale =
			Clamp(m_entityGhost_scale + (ip->scrolledDown() - ip->scrolledUp()) * 0.1f, 0.f, 10.f);
	else
		m_entityGhost_rotationY = mod(
			m_entityGhost_rotationY + (ip->scrolledDown() - ip->scrolledUp()) * 0.1f, 2 * XM_PI);
	// _Mesh List_
	static const ImVec2 imgSize = ImVec2(75, 75);
	if (ImGui::BeginChild(
			"Mesh List", ImVec2(imgSize.x * 3 + 75, 0), true, ImGuiWindowFlags_AlwaysAutoResize)) {
		for (size_t i = 0; i < m_entityViews.size(); i++) {
			bool selected = (i == m_selectedEntity);
			ImGui::BeginGroup();
			ImVec2 wrappedTextSize =
				ImGui::CalcTextSize(m_entityViews[i].objName.c_str(), NULL, false, imgSize.x);
			ImGui::BeginChild(1000 + i, wrappedTextSize, false, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::TextWrapped(m_entityViews[i].objName.c_str());
			ImGui::EndChild();
			if (selected) {
				ImGui::PushStyleColor(21, ImVec4(1, 1, 0, 1));
				ImGui::PushStyleColor(22, ImVec4(0.75f, 0.75f, 0, 1));
			}
			if (ImGui::ImageButton(m_entityViews[i].layer.getSRV().Get(), imgSize)) {
				// select mesh
				if (selected) {
					m_selectedEntity = -1;
				}
				else {
					m_selectedEntity = i;
					m_entityGhost_placer.load(m_entityViews[i].objName);
				}
			}
			if (ImGui::IsItemHovered()) {
				m_entityViews[i].rotation.y += 1.f * scene->getDeltaTime();
				m_entityViews[i].render = true;
			}
			if (selected)
				ImGui::PopStyleColor(2);
			ImGui::EndGroup();
			if ((i + 1) % 3 != 0)
				ImGui::SameLine();
		}
	}
	ImGui::EndChild();
}

void SceneEditorManager::update_panel_terrain_unselected() {}

void SceneEditorManager::update_panel_sea_unselected() {}

void SceneEditorManager::update_panel_ps_unselected() {
	vector<shared_ptr<ParticleSystem::ParticleDescription>>* list =
		ParticleSystem::GetDescriptionList();
	static string psdName = "";
	if (ImGui::Button("Create New")) {
		if (psdName != "") {
			// check if already exists
			bool exists = false;
			for (size_t i = 0; i < list->size(); i++) {
				if (list->at(i)->identifier == psdName) {
					// already exists
					exists = true;
					break;
				}
			}
			// create new description if none exists
			if (!exists) {
				ParticleSystem::ParticleDescription desc;
				desc.save(psdName);
				ParticleSystem::ReadDescriptionList();
			}
			else
				ErrorLogger::logWarning(
					"A ParticleDescription with the name \"" + psdName + "\" already exists!");
		}
	}
	ImGui::SameLine();
	ImGui::InputText("PSD Name", &psdName);
	ImGui::Separator();
	static int selectedIdx = 0;
	static shared_ptr<ParticleSystem::ParticleDescription> desc = list->at(0);
	string preview = desc.get() == nullptr ? "Empty List" : desc->identifier;
	struct Funcs {
		static bool ItemGetter(void* data, int n, const char** out_str) {
			*out_str =
				((shared_ptr<ParticleSystem::ParticleDescription>*)data)[n]->identifier.c_str();
			return true;
		}
	};
	if (ImGui::Combo("Description Templates", &selectedIdx, &Funcs::ItemGetter, list->data(),
			list->size(), 25)) {
		desc = list->at(selectedIdx);
	}
	ImGui::Separator();
	desc->imgui_properties();
	if (ImGui::Button("Save")) {
		desc->save();
	}
	if (0) {
		// use when updating description struct and need to save all descriptions
		ImGui::SameLine();
		if (ImGui::Button("Save All")) {
			vector<shared_ptr<ParticleSystem::ParticleDescription>>* list =
				ParticleSystem::GetDescriptionList();
			for (size_t i = 0; i < list->size(); i++)
				list->at(i)->save();
		}
	}
}

void SceneEditorManager::update_panel_effect_unselected() {}

void SceneEditorManager::update_panel_worldMessage_unselected() {}

SceneEditorManager::SceneEditorManager() {
	setPlayerState(false);
	TextureRepository* tr = TextureRepository::getInstance();
	// terrain heightmap textures
	TextureRepository::Type type = TextureRepository::Type::type_heightmap;
	m_heightmap_textures.push_back(tr->get("DesertMap.png", type));
	m_heightmap_textures.push_back(tr->get("flatmap.jpg", type));
	m_heightmap_textures.push_back(tr->get("ForestMap.png", type));
	m_heightmap_textures.push_back(tr->get("PlainMap.png", type));
	m_heightmap_textures.push_back(tr->get("tutorial.png", type));
	m_heightmap_textures.push_back(tr->get("VolcanoMap.png", type));

	// loadable entities from name file

	vector<string> loadable;
	ifstream nameFile;
	nameFile.open("assets/meshNames.txt", ios::in);
	if (nameFile.is_open()) {
		string line;
		while (getline(nameFile, line)) {
			loadable.push_back(line);
		}
	}
	nameFile.close();

	m_entityViews.resize(loadable.size());
	for (size_t i = 0; i < m_entityViews.size(); i++) {
		m_entityViews[i].objName = loadable[i];
		m_entityViews[i].layer.set(XMUINT2(500, 500), DXGI_FORMAT_R32G32B32A32_FLOAT,
			D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
	}


	// pointer object
	m_pointer_obj.load("arrow");
	m_pointer_obj.setScale(0.8);

	// crosshair
	m_crosshair.load("crosshair_blackCross.png");
	m_crosshair.set(float2(1280. / 2, 720. / 2), float2(1. / 30));
	m_crosshair.setAlignment(); // center - center

	// translation
	for (size_t i = 0; i < 3; i++) {
		m_arrow[i].load("arrow");
	}
	m_arrow[0].rotateY(XM_PI / 2.f);
	m_arrow[1].rotateX(XM_PI / 2.f);
	m_centerOrb.load("sphere");
	// rotation
	m_torus[0].load("torusX");
	m_torus[1].load("torusY");
	m_torus[2].load("torusZ");
	m_rotationCircle.load("sphere");
	// scaling
	m_scaling_torus.load("torusY");

	readSceneDirectory();
}

void SceneEditorManager::update(double dt) {
	Input* ip = Input::getInstance();

	scene->updateTime(dt);

	// update camera
	updateCameraMovement(dt);

	// Update Skybox
	scene->m_skyBox.update(dt);

	// update AreaTag
	const Environment* activeEnvironment =
		scene->m_terrains.getTerrainFromPosition(scene->m_camera.getPosition());
	if (activeEnvironment != nullptr) {
		AreaTag tag = activeEnvironment->getTag();
		scene->update_activeTerrain(tag, false);
	}

	// update water
	for (size_t i = 0; i < scene->m_seaEffects.size(); i++) {
		scene->m_seaEffects[i]->update(dt);
	}

	// particle system
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i]->update(dt);
	}

	// world message
	for (size_t i = 0; i < scene->m_worldMessages.size(); i++)
		scene->m_worldMessages[i]->update(dt);

	// effects
	for (size_t i = 0; i < scene->m_effects.size(); i++) {
		scene->m_effects[i]->update(dt);
	}

	////////////EDITOR///////////

	update_imgui();

	if (m_editorTabActive == EditorTab::Library || m_editorTabActive == EditorTab::GameRules) {
		// pick position
		if (ip->mousePressed(Input::RIGHT)) {
			float3 position = scene->m_camera.getPosition();
			float3 forward;
			if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
				// x coordinate is backwards!! Not because of mouse position but rather
				// getMousePickVector return
				float2 mpos = ip->mouseXY() / float2(1280, 720);
				mpos.x = 1 - mpos.x;
				forward = scene->m_camera.getMousePickVector(mpos) * m_pointer_range;
			}
			else {
				forward = Normalize(scene->m_camera.getForward()) * m_pointer_range;
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
					m_pointer_normal = scene->m_terrains.getNormalFromPosition(point);
				}
			}
			// entities
			for (size_t i = 0; i < scene->m_entities.size(); i++) {
				float3 cast_target, cast_normal;
				if (scene->m_entities[i]->castRayEx_limitDistance(
						position, forward, cast_target, cast_normal)) {
					float tt = (cast_target - position).Length();
					if (closest == -1 || tt < closest) {
						closest = tt;
						m_pointer = cast_target;
						m_pointer_normal = cast_normal;
					}
				}
			}
		}
	}

	if (m_editorTabActive == EditorTab::Library) {
		// pick fragment
		if (ip->mousePressed(Input::MIDDLE)) {
			float3 position = scene->m_camera.getPosition();
			float3 forward;
			if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
				// x coordinate is backwards!! Not because of mouse position but rather
				// getMousePickVector return
				float2 mpos = ip->mouseXY() / float2(1280, 720);
				mpos.x = 1 - mpos.x;
				forward = scene->m_camera.getMousePickVector(mpos) * m_pointer_range;
			}
			else {
				forward = Normalize(scene->m_camera.getForward()) * m_pointer_range;
			}
			Fragment* f = nullptr;
			float closest = -1;
			// terrain
			for (size_t i = 0; i < scene->m_terrains.size(); i++) {
				float t = scene->m_terrains[i]->castRay(position, forward);
				if (t != -1 && (closest == -1 || t < closest)) {
					closest = t;
					f = scene->m_terrains[i].get();
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
			// select in library
			if (f != nullptr) {
				select_fragment(f->getID());
			}
		}
		update_transformation(dt);

		// entity placer (entity tab open, no fragment selected, a mesh selected)
		if (m_libraryTabOpen == LibraryTab::tab_entity && m_library_selections[tab_entity] == -1 &&
			m_selectedEntity != -1) {
			float3 position = scene->m_camera.getPosition();
			float3 forward;
			if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
				// x coordinate is backwards!! Not because of mouse position but rather
				// getMousePickVector return
				float2 mpos = ip->mouseXY() / float2(1280, 720);
				mpos.x = 1 - mpos.x;
				forward = scene->m_camera.getMousePickVector(mpos) * m_pointer_range;
			}
			else {
				forward = Normalize(scene->m_camera.getForward()) * m_pointer_range;
			}
			// find closest collision point
			float closest = -1;
			float3 closestPoint;
			// terrain
			float t = scene->m_terrains.castRay(position, forward);
			if (t != -1) {
				float3 point = position + forward * t;
				float diff = (point - position).Length();
				if (closest == -1 || diff < closest) {
					closest = diff;
					closestPoint = point;
				}
			}
			// entities
			for (size_t i = 0; i < scene->m_entities.size(); i++) {
				float3 cast_target, cast_normal;
				if (scene->m_entities[i]->castRayEx_limitDistance(
						position, forward, cast_target, cast_normal)) {
					float diff = (cast_target - position).Length();
					if (closest == -1 || diff < closest) {
						closest = diff;
						closestPoint = cast_target;
					}
				}
			}
			if (closest != -1) {
				// update transformation
				m_entityGhost_placer.setPosition(closestPoint);
				m_entityGhost_placer.setRotation(float3(0, m_entityGhost_rotationY, 0));
				m_entityGhost_placer.setScale(m_entityGhost_scale);

				// placing
				if (ip->mousePressed(Input::MouseButton::LEFT)) {
					shared_ptr<Entity> obj = make_shared<Entity>(m_entityGhost_placer);
					scene->m_entities.add(obj->getLocalBoundingBoxPosition(),
						obj->getLocalBoundingBoxSize(), obj->getMatrix(), obj);
					// randomize
					entityGhost_randomizeProperties();
				}
			}
		}
	}

	// brush
	if (m_editorTabActive == EditorTab::TerrainEditor) {
		// pick on terrain
		float3 point = scene->m_camera.getPosition();
		float3 forward = scene->m_camera.getForward() * 100; // MOUSE RELATIVE MODE
		if (ip->getMouseMode() == DirectX::Mouse::MODE_ABSOLUTE) {
			float2 mpos = ip->mouseXY() / float2(1280, 720);
			mpos.x = 1 - mpos.x;
			forward = scene->m_camera.getMousePickVector(mpos) * m_pointer_range;
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
			scene->m_terrains.editMesh(m_terrainBrush, Brush::Type::Raise);
		if (ip->mouseDown(m_terrainEditor_btn_lower))
			scene->m_terrains.editMesh(m_terrainBrush, Brush::Type::Lower);
		if (ip->mouseDown(m_terrainEditor_btn_flatten))
			scene->m_terrains.editMesh(m_terrainBrush, Brush::Type::Flatten);
		// undo mesh
		if (ip->keyPressed(m_terrainEditor_btn_undo)) {
			scene->m_terrains.editMesh_pop();
		}
		// scroll values
		float radiusChangeOnMouseWheel = 1.1;
		if (ip->scrolledDown()) {
			if (ip->keyDown(m_terrainEditor_btn_strengthScroll))
				m_terrainBrush.strength =
					Clamp<float>(m_terrainBrush.strength * radiusChangeOnMouseWheel, 0, 1);
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

void SceneEditorManager::entityGhost_randomizeProperties() {
	if (m_entityGhost_randomRotation)
		m_entityGhost_rotationY = RandomFloat(0, 2 * XM_PI);
	if (m_entityGhost_randomScale)
		m_entityGhost_scale = RandomFloat(m_entityGhost_scaleRange.x, m_entityGhost_scaleRange.y);
}

void SceneEditorManager::render_entityViews() {
	float3 edges[8] = { float3(1, 1, 1), float3(1, 1, -1), float3(1, -1, 1), float3(1, -1, -1),
		float3(-1, 1, 1), float3(-1, 1, -1), float3(-1, -1, 1), float3(-1, -1, -1) };
	Camera camera;
	camera.setEye(float3(10, 5, 10) * 0.2f);
	camera.setAspectRatio(1);
	camera.setTarget(float3(0, 0, 0));
	camera.bind();
	vector<FrustumPlane> fplanes = camera.getFrustumPlanes();
	vector<float3> frustumPoints = camera.getFrustumPoints(0.4f);
	Renderer::getInstance()->setDrawState(Renderer::DrawingState::state_normal);
	for (size_t i = 0; i < m_entityViews.size(); i++) {
		if (m_entityViews[i].render) {
			m_entityViews[i].render = false;

			Renderer::getDeviceContext()->OMSetRenderTargets(1,
				m_entityViews[i].layer.getRTV().GetAddressOf(),
				Renderer::getInstance()->getDepthStencilView().Get());

			D3D11_VIEWPORT vp;
			vp.Width = 500;
			vp.Height = 500;
			vp.MinDepth = 0.0f;
			vp.MaxDepth = 1.0f;
			vp.TopLeftX = 0;
			vp.TopLeftY = 0;
			Renderer::getDeviceContext()->RSSetViewports(1, &vp);

			float clearColor[] = { 0.25f, .5f, 1, 1 };
			Renderer::getDeviceContext()->ClearRenderTargetView(
				m_entityViews[i].layer.getRTV().Get(), clearColor);
			Renderer::getDeviceContext()->ClearDepthStencilView(
				Renderer::getInstance()->getDepthStencilView().Get(), D3D11_CLEAR_DEPTH, 1, 0);

			ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
			shadowMap->mapShadowToFrustum(frustumPoints); // optimize shadowed area
			Renderer::getInstance()->getShadowMapper()->setup_shadowRendering();

			m_entityViewer.load(m_entityViews[i].objName);
			m_entityViewer.setPosition(float3(0, 0, 0));
			m_entityViewer.setRotation(float3(0, 0, 0));
			m_entityViewer.setScale(1);
			float3 bb_pos = m_entityViewer.getLocalBoundingBoxPosition();
			float3 bb_size = m_entityViewer.getLocalBoundingBoxSize();
			float3 points[8];
			float minT = -1;
			for (size_t i = 0; i < 8; i++) {
				points[i] = bb_size * edges[i];
				for (size_t f = 0; f < fplanes.size(); f++) {
					float t = fplanes[f].castRay(float3(0, 0, 0), points[i]);
					if (t > 0 && (minT == -1 || t < minT)) {
						minT = t;
					}
				}
			}
			m_entityViewer.setScale(minT);
			m_entityViewer.setRotation(m_entityViews[i].rotation);
			m_entityViewer.setPosition(-m_entityViewer.getLocalBoundingBoxPosition() * minT);
			scene->m_skyBox.bindLightBuffer();
			m_entityViewer.draw();
		}
	}
}

void SceneEditorManager::draw_shadow() { // terrain manager
	ShadowMapper* shadowMap = Renderer::getInstance()->getShadowMapper();
	vector<FrustumPlane> planes = shadowMap->getFrustumPlanes();

	// draw terrainBatch
	scene->m_terrains.quadtreeCull(planes);
	scene->m_terrains.draw_onlyMesh();

	// terrain entities
	vector<shared_ptr<Entity>*> culledEntities = scene->m_entities.cullElements(planes);
	for (size_t i = 0; i < culledEntities.size(); i++)
		(*culledEntities[i])->draw_onlyMesh(float3(0.f));

	// Editor Stuff
	if (m_libraryTabOpen == LibraryTab::tab_entity && m_library_selections[tab_entity] == -1 &&
		m_selectedEntity != -1) {
		m_entityGhost_placer.draw_onlyMesh();
	}
}

void SceneEditorManager::draw_color() {
	// frustum data for culling
	vector<FrustumPlane> frustum = scene->m_camera.getFrustumPlanes();
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

	// Editor Stuff
	if (m_libraryTabOpen == LibraryTab::tab_entity && m_library_selections[tab_entity] == -1 &&
		m_selectedEntity != -1) {
		m_entityGhost_placer.draw();
	}

	// SkyBox
	scene->m_skyBox.draw();

	// Dark edges
	Renderer::getInstance()->draw_darkEdges();

	/* --- Things to be drawn without dark edges --- */

	// terrain grass
	if (m_editorTabActive != EditorTab::TerrainEditor) // hide grass when editing heightmap
		scene->m_terrains.draw_grass();

	// Particle Systems
	for (size_t i = 0; i < scene->m_particleSystems.size(); i++) {
		scene->m_particleSystems[i]->draw();
	}
	// effects
	for (size_t i = 0; i < scene->m_effects.size(); i++) {
		scene->m_effects[i]->draw();
	}

	m_tex3d.setText("Hello World");
	m_tex3d.setColor(Color(1, 0, 0, 1));
	m_tex3d.draw();
}

void SceneEditorManager::draw_hud() { m_crosshair.draw(); }

void SceneEditorManager::draw_editorWorldObjects() {
	// Clear depth so that all entities are drawn infront of everything else
	// but still has depth perception to other editor entities.
	Renderer::getInstance()->clearDepth();
	// Draw
	if (m_editorTabActive == EditorTab::Library) {
		// Pointer Entity Helper
		m_pointer_obj.setPosition(m_pointer);
		m_pointer_obj.setRotation(vector2Rotation(-m_pointer_normal));
		m_pointer_obj.draw_onlyMesh(float3(1, 0.5, 0.5));
		// Transformation Entity Helpers
		draw_transformationVisuals();
	}
	if (m_editorTabActive == EditorTab::GameRules) {
		// Pointer Entity Helper
		m_pointer_obj.setPosition(m_pointer);
		m_pointer_obj.setRotation(vector2Rotation(-m_pointer_normal));
		m_pointer_obj.draw_onlyMesh(float3(1, 0.5, 0.5));
	}
}

void SceneEditorManager::draw() {

	render_entityViews();

	setup_shadow();
	draw_shadow();
	setup_color();
	draw_color();
	draw_finalize();
	draw_editorWorldObjects();
	draw_hud();
}

void SceneEditorManager::load(string folder) {
	clear();

	SceneManager::load(folder);
}

void SceneEditorManager::reset() { scene->reset(); }

void SceneEditorManager::clear() {
	m_editorTabActive = EditorTab::Library;
	m_libraryTabOpen = LibraryTab::tab_terrain;
	m_transformable = nullptr;
	for (size_t i = 0; i < LibraryTab::tab_count; i++)
		m_library_selections[i] = -1;
	scene->clear();
}
