#pragma once
#include "StateItem.h"
#include "TextRenderer.h"
#include "Entity.h"
#include "Button.h"
#include "ParticleSystem.h"
#include "Timer.h"
#include "Camera.h"
#include "Sprite2D.h"
#include "Translation2DStructures.h"

class EndRoundState : public StateItem {
private:
	//Camera m_camera;
	//Timer m_timer;
	//Sprite2D m_background;
	//TextRenderer m_textRenderer;
	//string m_victoryText;
	//float4 m_victoryColor;
	//string m_timeText;
	//Button m_exitButton;
	//Button m_levelSelectButton;
	//Button m_restartButton;
	//ParticleSystem m_particleSystem;
	//Entity m_bowlContent;
	//Entity m_bowl;
	//size_t m_currentBowlContent;

	Timer m_timer;

	float m_time_max;
	float m_time_previous;
	float m_time_current;

	//background
	float m_background_offset = 0;
	float m_background_offset_speed = 0.15f;
	float2 m_background_offset_direction = float2(1,-1);
	Sprite2D m_background;
	Sprite2D m_bar_back, m_bar_pre, m_bar_curr;
	
	//header
	TextRenderer m_text_header;

	//panel
	Sprite2D m_panel;

	//medals
	struct Medal {
		const float gravity = 1000;
		bool moving = false;
		float2 startPosition;
		float2 position,velocity;
		float rotation = 0;
		float rotation_velocity = 0;
		float time;
		Sprite2D sprite, sprite_back;
		void activate() { 
			if (!moving) {
				float dir = (-3.1415f / 2) + RandomFloat(-1, 1) * (3.1415f / 4);
				velocity = float2(cos(dir), sin(dir)) * RandomFloat(1, 1)*400;
				rotation_velocity = RandomFloat(-1, 1) * 5;
				moving = true;
				ErrorLogger::log("Activate");
			}
		}
		void setPosition(float2 _position) { 
			startPosition = _position;
			position = _position;
		}
		void setSize(float size) { 
			sprite.setSize(float2(1.) * size);
			sprite_back.setSize(float2(1.) * size);
		}
		void update(float dt) { 
			if (moving) {
				velocity.y += gravity * dt;
				position += velocity * dt;
				rotation += rotation_velocity * dt;
			}
		}
		void draw_back() {
			sprite_back.setPosition(startPosition);
			sprite_back.draw();
		}
		void draw_front() { 
			sprite.setPosition(position);
			sprite.setRotation(rotation);
			sprite.draw();
		}
		void load(string texture, float _time) {
			time = _time;
			sprite.load(texture);
			sprite_back.load("coin_holder.png");
			sprite.setAlignment();//center
			sprite_back.setAlignment();//center
		}
	};
	Medal m_medals[NR_OF_TIME_TARGETS];
	TextRenderer m_text_pointer;
	Sprite2D m_barPointer;
	float m_bar_max = 400;
	float m_bar_height = 40;
	float m_bar_timer = 0;
	float m_bar_progress = 0;

	//stars
	struct CollectedStar {
		float2 position;
		Sprite2D sprite;
		TextRenderer text;
		int count;
		void draw() { 
			sprite.setPosition(position);
			sprite.draw();
			text.draw(to_string(count), position + float2(sprite.getSize().x / 2 + 10, 0));
		}
		void load(float2 _position, Color color, int _count) {
			position = _position;
			sprite.load("circle_white.png");
			sprite.setSize(float2(1.)*80);
			sprite.setAlignment();
			sprite.setColor(color);
			text.setAlignment(TextRenderer::HorizontalAlignment::LEFT);
			count = _count;
		}
	};
	CollectedStar m_stars[NR_OF_TIME_TARGETS];

	//success/failure
	TextRenderer m_text_success;
	float m_success_timer = 0;

	//conffeti
	Sprite2D m_confettiBody;
	struct Confetti {
		Projectile transform;
		Color m_color;
		void update(float dt) { 
			transform.update(dt);
		}
		void draw(Sprite2D* body) { 
			body->setPosition(transform.getPosition());
			body->setColor(m_color);
			body->setSize(transform.getScale());
			body->setRotation(transform.getRotation());
			body->draw();
		}
		Confetti(float2 position, float2 size, float rotVelocity, Color color) { 
			transform.setPosition(position);
			transform.setScale(size);
			transform.setMass(size.x * size.y *0.001f);
			transform.setRotationVelocity(rotVelocity);
			transform.setGravity(float2(0, 2000));
			m_color = color;
		}
		Confetti() {}
	};
	vector<Confetti> m_confetti;
	bool m_confettiShot = false;

	// -- Private Functions -- 
	bool isVictorious() const;
	void shootConfetti();

public:
	EndRoundState();
	~EndRoundState();

	void init();
	void update();
	void draw();

	void pause();
	void play();
	void restart();

	void analyseScene();

	//void setParticleColorByPrize(size_t prize);
	//void setVictoryText(string text);
	//void setVictoryColor(float4 color);
	//void setBowl(string bowlContentEntityName, int bowlMaterial);
	//void setTimeText(string text);
	//void setConfettiPower(float emitRate);

};
