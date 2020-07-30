#pragma once
#include "SceneManager.h"
#include "RPYCamera.h"
#include "Sprite2D.h"
#include "TextureRepository.h"

/* Handles editing and drawing of scene data */
class SceneEditorManager : public SceneManager {
private:
	RPYCamera m_camera;
	float3 m_cam_velocity;
	float m_cam_friction = 0.7;
	float m_highSpeed = 60;
	float m_lowSpeed = 25;

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
	const Keyboard::Keys m_key_resetTransform = Keyboard::R; // btn for nulling transform

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
	float3 m_target_rot;
	float3 m_target_forward;
	float m_target_rayDist;

	ImTextureID m_image;

	const float m_pointer_range = 100;
	float3 m_pointer;
	Entity m_pointer_obj;

	// Resources
	vector<shared_ptr<TextureSet>> m_heightmap_textures;
	vector<shared_ptr<TextureSet>> m_terrain_textures;

	vector<string> m_loadable_entity;

	Entity m_anim_face_obj;
	Entity m_animationTest;
	enum AnimationState {
		bouncing,
		onGround
	} m_anim_state = onGround;
	float3 m_anim_position, m_anim_velocity, m_lookPosition = float3(1,0,0);
	float m_anim_heightAboveGround = 0.5;
	float3 m_anim_gravity_dir = float3(0, -1, 0);
	float m_anim_gravity_strength = 20;
	float m_anim_jump_strength = 10;
	float m_anim_groundHitSlowdown = 0.5;
	float m_anim_bounce_des = 1;
	float m_anim_bounce_pos = 1;
	float m_anim_bounce_vel = 0;
	float m_anim_bounce_speed = 500;
	float m_anim_bounce_slowdown = 0.026;
	float m_anim_bounce_onJumpEffect = 5;
	float m_anim_bounce_onHitEffect = 0.5;
	float m_anim_lookat_speed = 6;
	float m_faceDistance = 0.7;

	//-- Private Functions --

	void updateCameraMovement(float dt);

	void update_transformation(float dt);
	void draw_transformationVisuals();

	void update_imgui();

public:
	SceneEditorManager();

	void update();

	void draw_shadow();
	void draw_color();
	void draw_hud();
	void draw();

	void load(string folder);

};
