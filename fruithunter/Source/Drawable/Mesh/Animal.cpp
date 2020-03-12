#include "Animal.h"
#include "Renderer.h"
#include "AudioHandler.h"
#define START_ZOOM_LENGTH 1500

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
		// lays down
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
	m_hasAttacked = false; // for triggering angry sound once when inside attack radius
	m_isSatisfied = false; // for triggering happy sound once when briebed.
	m_isLookedAt = false;
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

	m_thoughtBubbleOffset = getBoundingBoxPos() + float3(0.f, getHalfSizes().y + 0.5f, 0.f);
	float3 topPos = getPosition() + m_thoughtBubbleOffset;
	m_thoughtBubble.setPosition(topPos);
}

Animal::~Animal() {}

float Animal::getThrowStrength() const { return m_throwStrength; }

float Animal::getPlayerRange() const { return m_playerRange; }

float Animal::getFruitRange() const { return m_fruitRange; }

int Animal::getfruitType() const { return m_fruitType; }

bool Animal::notBribed() const { return m_nrFruitsTaken < m_nrRequiredFruits; }

void Animal::setAttacked(bool attacked) { m_hasAttacked = attacked; }

void Animal::makeAngrySound() {
	auto audioHandler = AudioHandler::getInstance();
	string animal = getModelName();
	if (!m_hasAttacked) {
		if (animal == "Bear") {
			audioHandler->playOnce(AudioHandler::BEAR_PUSH);
		}
		if (animal == "Goat") {
			audioHandler->playOnce(AudioHandler::GOAT_PUSH);
		}
		if (animal == "Gorilla") {
			audioHandler->playOnce(AudioHandler::GORILLA_PUSH);
		}
		m_hasAttacked = true;
	}
}

void Animal::makeHappySound() {
	auto audioHandler = AudioHandler::getInstance();
	string animal = getModelName();
	if (!m_isSatisfied) {
		if (animal == "Bear") {
			audioHandler->playOnce(AudioHandler::BEAR_HAPPY);
		}
		if (animal == "Goat") {
			audioHandler->playOnce(AudioHandler::GOAT_HAPPY);
		}
		if (animal == "Gorilla") {
			audioHandler->playOnce(AudioHandler::GORILLA_HAPPY);
		}
		m_isSatisfied = true;
	}
}

void Animal::makeEatingSound() {
	auto audioHandler = AudioHandler::getInstance();
	string animal = getModelName();

	if (animal == "Bear") {
		audioHandler->playOnce(AudioHandler::BEAR_EATING);
	}
	if (animal == "Goat") {
		audioHandler->playOnce(AudioHandler::GOAT_EATING);
	}
	if (animal == "Gorilla") {
		audioHandler->playOnce(AudioHandler::GORILLA_EATING);
	}
}


void Animal::grabFruit(float3 pos) {
	m_walkToPos = pos;
	m_walkTimeTracker = 0;
	m_nrFruitsTaken++;
	int fruitsLeft = min(m_nrRequiredFruits - m_nrFruitsTaken, 3);
	m_thoughtBubble.setCurrentMaterial(3 * m_fruitType + fruitsLeft - 1);
	if (!notBribed()) {
		m_thoughtBubble.setCurrentMaterial(9);
		m_thoughtBubbleOffset.y = 1.5f;
		makeHappySound();
	}
	else {
		makeEatingSound();
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
		else {
			walkToSleep(dt);
		}
	}
	else if (notBribed())
		setRotation(float3(0.f, m_startRotation, 0.0f));

	// update thoughtBubble

	m_thoughtBubble.lookTo(playerPos);
	float scaleMax = 4.5f;
	float scaleFactor = m_thoughtBubble.getScale().x;
	scaleFactor = (scaleFactor - 1.f) / (scaleMax - 1.f);
	scaleFactor += dt * ((float)m_isLookedAt - 0.5f) * 12.f;
	scaleFactor = min(1.f, max(0.f, scaleFactor));
	m_thoughtBubble.setScale(scaleFactor * (scaleMax - 1.f) + 1.f);

	float3 topPos = getPosition() + m_thoughtBubbleOffset;
	float3 topPos2 = topPos + float3(0.f, 1.8f, 0.f);
	m_thoughtBubble.setPosition(XMVectorLerp(topPos, topPos2, scaleFactor));
}

void Animal::beginWalk(float3 pos) {
	if (m_walkTimeTracker >= 2) {
		m_walkToPos = (pos + getPosition()) * 0.5;
		m_walkTimeTracker = 0.f;
	}
}

bool Animal::checkLookedAt(float3 playerPos, float3 rayDir) {
	float3 playerToAnimal = getPosition() + getBoundingBoxPos() - playerPos;
	bool isLookedAt = false;
	if (playerToAnimal.LengthSquared() < START_ZOOM_LENGTH) {
		float3 vec = rayDir.Cross(playerToAnimal);
		vec = vec.Cross(rayDir);
		vec.Normalize();
		float proj = playerToAnimal.Dot(vec);

		isLookedAt = abs(proj) < getHalfSizes().x * 1.5f;
	}
	m_isLookedAt = isLookedAt;
	return isLookedAt;
}
