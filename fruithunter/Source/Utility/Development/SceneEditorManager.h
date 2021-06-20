#pragma once
#include "SceneManager.h"
#include "RPYCamera.h"
#include "Sprite2D.h"
#include "TextureRepository.h"
#include "filesystemHelper.h"
#include "CSteamLeaderboards.h"

/* Handles editing and drawing of scene data */
class SceneEditorManager : public SceneManager {
private:
	CSteamLeaderboard m_leaderboard;

	float3 m_cam_velocity;
	float m_cam_friction = 0.7f;
	float m_highSpeed = 60.f;
	float m_lowSpeed = 25.f;

	// Edit Transform
	Transformation* m_transformable = nullptr;

	enum EditTransformState {
		Edit_Translate,
		Edit_Scaling,
		Edit_Rotation,
		Edit_Count
	} m_transformState = Edit_Translate;
	// KeyBindings
	const Keyboard::Keys m_key_delete = Keyboard::Delete;
	const Input::MouseButton m_key_target = Input::LEFT; // btn for targeting transformation options
	const Input::MouseButton m_key_select = Input::MIDDLE;	// btn for selecting entity
	const Keyboard::Keys m_key_switchState = Keyboard::Tab; // btn for selecting entity
	const Keyboard::Keys m_key_copy = Keyboard::C;			// btn for copying fragment
	const Keyboard::Keys m_key_setPosition =
		Keyboard::V; // btn for overwriting position to pointer pos

	const Keyboard::Keys KEY_FORWARD = Keyboard::W;
	const Keyboard::Keys KEY_BACKWARD = Keyboard::S;
	const Keyboard::Keys KEY_LEFT = Keyboard::A;
	const Keyboard::Keys KEY_RIGHT = Keyboard::D;
	const Keyboard::Keys KEY_UP = Keyboard::Space;
	const Keyboard::Keys KEY_DOWN = Keyboard::LeftControl;
	const Keyboard::Keys KEY_SLOW = Keyboard::LeftShift;

	// crosshair
	Sprite2D m_crosshair;

	// PYR rotation mask
	float3 m_maskPYR[3] = { float3(1, 1, 0), float3(0, 1, 0),
		float3(1, 1, 1) }; // masks rotation axises that is desired for a torus

	// axises
	float3 m_axis[3] = { float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1) };

	// entities for transformation
	Entity m_arrow[3], m_centerOrb;		 // translation
	Entity m_torus[3], m_rotationCircle; // rotation
	Entity m_scaling_torus;				 // scaling

	// on tranformation variables
	int m_target = -1;
	float3 m_target_pos; // camera
	float3 m_target_forward;
	float m_target_rayDist;

	const float m_pointer_range = 100;
	float3 m_pointer, m_pointer_normal;
	Entity m_pointer_obj;

	// Resources
	vector<shared_ptr<TextureSet>> m_heightmap_textures;

	vector<string> m_loadable_entity;
	struct EntityView {
		bool render = true;
		string objName;
		float3 rotation;
		Layer layer;
	};
	vector<EntityView> m_entityViews;
	Entity m_entityViewer;

	vector<string> m_loadable_scenes;

	int m_selectedEntity = -1;
	bool m_entityGhost_randomRotation = false;
	bool m_entityGhost_randomScale = false;
	float2 m_entityGhost_scaleRange = float2(1, 1);
	float m_entityGhost_scale = 1.f;
	float m_entityGhost_rotationY = 0;
	Entity m_entityGhost_placer;

	enum LibraryTab {
		tab_terrain,
		tab_entity,
		tab_sea,
		tab_particleSystem,
		tab_count
	} m_libraryTabOpen = LibraryTab::tab_terrain;
	int m_library_selections[tab_count] = { -1 };

	enum EditorTab {
		Library,
		GameRules,
		TerrainEditor,
		Leaderboard,
		etab_count
	} m_editorTabActive = EditorTab::Library;

	// terrain brush
	Brush m_terrainBrush;
	Keyboard::Keys m_terrainEditor_btn_strengthScroll = Keyboard::LeftShift;
	Keyboard::Keys m_terrainEditor_btn_undo = Keyboard::Z;
	Input::MouseButton m_terrainEditor_btn_lower = Input::MouseButton::RIGHT;
	Input::MouseButton m_terrainEditor_btn_raise = Input::MouseButton::LEFT;
	Input::MouseButton m_terrainEditor_btn_flatten = Input::MouseButton::MIDDLE;

	//-- Private Functions --

	void update_imgui_library();
	void update_imgui_gameRules();
	void update_imgui_terrainEditor();
	void update_imgui_leaderboard();

	void update_panel_terrain_unselected();
	void update_panel_entity_unselected();
	void update_panel_sea_unselected();
	void update_panel_ps_unselected();

	template <typename CLASS> void update_panel(vector<shared_ptr<CLASS>>& list, LibraryTab tab, void (SceneEditorManager::*func)(void));
	void update_panel_entity_improved(
		QuadTree<shared_ptr<Entity>>& list, LibraryTab tab, void (SceneEditorManager::*func)(void));

	void updateCameraMovement(float dt);

	void update_transformation(float dt);
	void draw_transformationVisuals();

	void update_imgui();
	void select_index(LibraryTab tab, int index);
	void select_fragment(FragmentID id);

	void readSceneDirectory();

	void render_entityViews();

	void entityGhost_randomizeProperties();

public:
	SceneEditorManager();

	void update();

	void draw_shadow();
	void draw_color();
	void draw_hud();
	void draw_editorWorldObjects();
	void draw();

	void load(string folder);
	void reset();
	void clear();
};

template <typename CLASS>
inline void SceneEditorManager::update_panel(
	vector<shared_ptr<CLASS>>& list, LibraryTab tab, void (SceneEditorManager::*func)(void)) {
	Input* ip = Input::getInstance();
	int& selectedIdx = m_library_selections[tab];
	if (ImGui::BeginChild(
			"FragmentDisplayer", ImVec2(200, 0), true, ImGuiWindowFlags_AlwaysAutoResize)) {
		// List
		ImVec2 panelSize = ImGui::GetWindowSize();
		if (ImGui::BeginChildFrame(3, ImVec2(0, panelSize.y - 135))) {
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
				// Drag Source
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
					ImGui::SetDragDropPayload("DND_TerrainItem", &i, sizeof(size_t));

					ImGui::Text("Dragging[%i]", i);
					ImGui::EndDragDropSource();
				}
				// Drag Target
				if (ImGui::BeginDragDropTarget()) {
					if (const ImGuiPayload* payload =
							ImGui::AcceptDragDropPayload("DND_TerrainItem")) {
						size_t payload_i = *(size_t*)payload->Data;
						shared_ptr<CLASS> payload_source = list[payload_i];
						list.erase(list.begin() + payload_i);
						list.insert(list.begin() + i, payload_source);
					}
					ImGui::EndDragDropTarget();
				}
				ImGui::PopID();
			}
			ImGui::EndChildFrame();
		}
		// buttons
		if (ImGui::Button("Create")) {
			list.push_back(make_shared<CLASS>());
		}
		ImGui::SameLine();
		if ((ImGui::Button("Remove") || ip->keyPressed(Keyboard::Delete)) && selectedIdx != -1) {
			list.erase(list.begin() + selectedIdx);
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("QuickButton (Del)");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
		if ((ImGui::Button("Duplicate") || ip->keyPressed(Keyboard::C)) && selectedIdx != -1) {
			list.push_back(make_shared<CLASS>(*list[selectedIdx].get()));
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
	{
		if (selectedIdx != -1) {
			static_cast<Fragment*>(list[selectedIdx].get())->imgui_properties();
		}
		else {
			std::invoke(func, *this);
		}
	}
	ImGui::EndGroup();
}
