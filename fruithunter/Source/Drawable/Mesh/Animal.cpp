#include "Animal.h"
#include "Renderer.h"

void Animal::walkAndBack(float dt) {
	if (m_walkTimeTracker < 1) { // on way to position
		setPosition(XMVectorLerp(m_origin, m_walkToPos, m_walkTimeTracker));
		m_walkTimeTracker += dt * m_chargeSpeed;
	}
	else { // on way back from position
		setPosition(XMVectorLerp(m_origin, m_walkToPos, 2 - m_walkTimeTracker));
		float3 vec = float3::Transform(float3::Forward, float4x4::CreateRotationY(m_startRotation));
		float angleSpeed = XM_PI * 3;
		float amplitude = 0.3f;
		setRotation(
			float3(0.f, m_startRotation, sin((m_walkTimeTracker - 1.f) * angleSpeed) * amplitude));
		m_walkTimeTracker += dt * m_returnSpeed;
	}
}
void Animal::walkToSleep(float dt) {
	if (m_walkTimeTracker < 0.5f) { // on way to position
		setPosition(XMVectorLerp(m_origin, m_walkToPos, m_walkTimeTracker * 2.f));
		m_walkTimeTracker += dt;
	}
	else if (m_walkTimeTracker < 1.f) { // on way home
		setPosition(XMVectorLerp(m_walkToPos, m_sleepPos, (m_walkTimeTracker - 0.5f) * 2.f));
		m_walkTimeTracker += dt;
	}
	else {
		// // lays down
		setRotation(float3(XM_PI * 0.5f * (m_walkTimeTracker - 1.f), m_startRotation, 0.f));
		m_walkTimeTracker += dt;
	}
}

Animal::Animal(string modelName, float playerRange, float fruitRange, int fruitType,
	int nrRequiredFruits, float throwStrength, float3 position, float3 sleepPos, float rotation)
	: Entity(modelName, position) {
	m_playerRange = playerRange;
	m_fruitRange = fruitRange;
	m_fruitType = fruitType;
	m_nrRequiredFruits = nrRequiredFruits;
	m_nrFruitsTaken = 0;
	m_throwStrength = throwStrength;
	m_origin = position;
	m_sleepPos = sleepPos;
	m_walkTimeTracker = 3;
	m_chargeSpeed = 6;
	m_returnSpeed = 1;
	rotateY(rotation);
	m_startRotation = rotation;

	// Thought bubble
	m_thoughtBubble.load("Quad");
	vector<string> mtlNames;
	mtlNames.resize(10);

	string fruits[] = { "Apple", "Banana", "Melon" };
	for (size_t i = 0; i < 10; ++i) {
		string mtlName = "ThoughtBubble" + to_string(i) + ".mtl";
		mtlNames[i] = mtlName;
	}
	m_thoughtBubble.loadMaterials(mtlNames, 10);
	m_thoughtBubble.setCurrentMaterial(3 * fruitType + nrRequiredFruits - 1);

	m_thoughtBubbleOffset = float3(0.f, getHalfSizes().y + 0.5f, 0.f);
	float3 topPos = getPosition() + getBoundingBoxPos() + m_thoughtBubbleOffset;
	m_thoughtBubble.setPosition(topPos);
}

Animal::~Animal() {}

float Animal::getThrowStrength() const { return m_throwStrength; }

float Animal::getPlayerRange() const { return m_playerRange; }

float Animal::getFruitRange() const { return m_fruitRange; }

int Animal::getfruitType() const { return m_fruitType; }

bool Animal::notBribed() const { return m_nrFruitsTaken < m_nrRequiredFruits; }

void Animal::grabFruit(float3 pos) {
	m_walkToPos = pos;
	m_walkTimeTracker = 0;
	m_nrFruitsTaken++;
	int fruitsLeft = min(m_nrRequiredFruits - m_nrFruitsTaken, 3);
	m_thoughtBubble.setCurrentMaterial(3 * m_fruitType + fruitsLeft - 1);
	if (!notBribed()) {
		m_thoughtBubble.setCurrentMaterial(9);
		m_thoughtBubbleOffset.y = 0.5;
	}
}

void Animal::draw() {
	Entity::draw(); // Draw the animal
					// Draw the thoughtbubble
	Renderer::getInstance()->enableAlphaBlending();
	m_thoughtBubble.draw();
	Renderer::getInstance()->disableAlphaBlending();
}

void Animal::update(float dt, float3 playerPos) {
	if (m_walkTimeTracker < 2) {
		if (notBribed())
			walkAndBack(dt);
		else
			walkToSleep(dt);

		// also update thoughtBubble
		float3 topPos = getPosition() + getBoundingBoxPos() + m_thoughtBubbleOffset;
		m_thoughtBubble.setPosition(topPos);
	}
	else if (notBribed())
		setRotation(float3(0.f, m_startRotation, 0.0f));

	// Billboard thoughtBubble
	m_thoughtBubble.lookTo(playerPos);
}

void Animal::beginWalk(float3 pos) {
	if (m_walkTimeTracker >= 2) {
		m_walkToPos = (pos + getPosition()) * 0.5;
		m_walkTimeTracker = 0.f;
	}
}
