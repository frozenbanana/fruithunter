#include "TotemLock.h"
#include "SceneManager.h"

TotemLock::TotemLock(float3 position, float rotationY) : Fragment(Fragment::Type::totemLock) {
	setPosition(position);
	setScale(1);
	setRotation(float3(0, rotationY, 0));
	m_obj_totem.load("totem");
}

void TotemLock::update(float dt) { 
	Scene* scene = SceneManager::getScene(); 

	float3 toPlayer = scene->m_player->getPosition() - getPosition();
	float distanceToPlayer = float2(toPlayer.x, toPlayer.z).Length(); // ignore y axis
	if (distanceToPlayer < m_activationRadius) {
		// player within radius
		// notify player with text on screen
		if (Input::getInstance()->keyPressed(Keyboard::E) && !m_activated) {
			m_activated = true;
		}
	}
	if (m_activated) {
		// when activated, remove totem using plane clipping.
		// also gradualy show the locked area by removing the gray filter
		//m_removalProgress = Clamp<float>(m_removalProgress + dt / m_removalTime, 0, 1);

		// create orb on top of totem in which collect fruits
	}
}

void TotemLock::draw() {
	m_obj_totem.setPosition(getPosition());
	m_obj_totem.setScale(getScale());
	m_obj_totem.setRotation(getRotation());

	if (m_activated) {
		//float totalTime = SceneManager::getScene()->m_timer.getTimePassed();
		//float3 bb_pos = m_obj_totem.getLocalBoundingBoxPosition() * m_obj_totem.getScale()+m_obj_totem.getPosition();
		//float3 bb_size = m_obj_totem.getLocalBoundingBoxSize()*m_obj_totem.getScale();
		//float3 planePos = bb_pos;
		//planePos.y += bb_size.y - (bb_size.y*2 + 0.001f) * m_removalProgress;
		//m_obj_totem.draw_clippingPlane(planePos, float3(0, 1, 0), m_removalPlaneColor, totalTime);


	}
	else {
		m_obj_totem.draw();
	}
	
	m_ps_collector.draw();
}
