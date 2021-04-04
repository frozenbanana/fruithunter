#pragma once
#include "SceneManager.h"
#include "RPYCamera.h"
#include "Sprite2D.h"
#include "TextureRepository.h"
#include "filesystemHelper.h"

/* Handles editing and drawing of scene data */
class SceneEditorManager : public SceneManager {
private:
	RPYCamera m_camera;
	float3 m_cam_velocity;
	float m_cam_friction = 0.7f;
	float m_highSpeed = 60.f;
	float m_lowSpeed = 25.f;

	//Edit Transform
	Transformation* m_transformable = nullptr;

	enum EditTransformState {
		Edit_Translate,
		Edit_Scaling,
		Edit_Rotation,
		Edit_Count
	} m_transformState = Edit_Translate;
	//KeyBindings
	const Keyboard::Keys m_key_delete = Keyboard::Delete;
	const Input::MouseButton m_key_target = Input::LEFT;// btn for targeting transformation options
	const Input::MouseButton m_key_select = Input::MIDDLE; // btn for selecting entity
	const Keyboard::Keys m_key_switchState = Keyboard::Tab; // btn for selecting entity
	const Keyboard::Keys m_key_copy = Keyboard::C; // btn for copying fragment
	const Keyboard::Keys m_key_setPosition = Keyboard::V; // btn for overwriting position to poiner pos

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
	float3 m_maskPYR[3] = { float3(1, 1, 0), float3(0, 1, 0), float3(1, 1, 1) };//masks rotation axises that is desired for a torus

	//axises
	float3 m_axis[3] = { float3(1, 0, 0), float3(0, 1, 0), float3(0, 0, 1) };

	//entities for transformation
	Entity m_arrow[3], m_centerOrb;//translation
	Entity m_torus[3], m_rotationCircle;//rotation
	Entity m_scaling_torus;//scaling

	//on tranformation variables
	int m_target = -1;
	float3 m_target_pos;//camera
	float3 m_target_forward;
	float m_target_rayDist;

	const float m_pointer_range = 100;
	float3 m_pointer, m_pointer_normal;
	Entity m_pointer_obj;

	// Resources
	vector<shared_ptr<TextureSet>> m_heightmap_textures;
	vector<shared_ptr<TextureSet>> m_terrain_textures;

	vector<string> m_loadable_entity;

	vector<string> m_loadable_scenes;

	vector<Fragment*> m_library;
	int m_selectedIndex = -1;
	bool m_selectedThisFrame = false;

	enum EditorTab {
		Library,
		GameRules,
		TerrainEditor 
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

	bool update_panel_terrain(Environment* selection, bool update = false);
	bool update_panel_entity(Entity* selection, bool update = false);
	bool update_panel_sea(SeaEffect* selection, bool update = false);
	bool update_panel_effect(ParticleSystem* selection, bool update = false);
	void refreshLibrary();

	void updateCameraMovement(float dt);

	void update_transformation(float dt);
	void draw_transformationVisuals();

	void update_imgui();
	void select_index(size_t index);
	void select_fragment(FragmentID id);
	void deselect_fragment();

	void readSceneDirectory();

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
