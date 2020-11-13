#include "Animal.h"
#include "Renderer.h"
#include "AudioController.h"
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

Animal::Animal(float3 position, float3 sleepPosition, Type type, FruitType fruitType, size_t nrRequiredFruits, float rotationY) : Entity("", position) {
	changeType(Fragment::Type::animal);
	m_type = type;
	m_origin = position;
	m_sleepPos = sleepPosition;
	m_startRotation = rotationY;
	m_fruitType = fruitType;
	m_nrRequiredFruits = nrRequiredFruits;
	rotateY(rotationY);
	setType(type);

	// Thought bubble
	m_thoughtBubble.load("Quad");
	vector<string> mtlNames;
	mtlNames.resize(10);

	string fruits[] = { "Apple", "Banana", "Melon" };
	for (size_t i = 0; i < 10; ++i) {
		string mtlName = "ThoughtBubble" + to_string(i) + ".mtl";
		mtlNames[i] = mtlName;
	}
	m_thoughtBubble.loadMaterials(mtlNames);
	m_thoughtBubble.setCurrentMaterial(3 * fruitType + nrRequiredFruits - 1);

	m_thoughtBubbleOffset =
		(getBoundingBoxPos() - getPosition()) + float3(0.f, getHalfSizes().y + 0.5f, 0.f);
	float3 topPos = getPosition() + m_thoughtBubbleOffset;
	m_thoughtBubble.setPosition(topPos);
}

Animal::~Animal() {}

float Animal::getThrowStrength() const { return m_throwStrength; }

float Animal::getPlayerRange() const { return m_playerRange; }

float Animal::getFruitRange() const { return m_fruitRange; }

FruitType Animal::getfruitType() const { return m_fruitType; }

int Animal::getRequiredFruitCount() const { return m_nrRequiredFruits; }

float3 Animal::getSleepPosition() const { return m_sleepPos; }

Animal::Type Animal::getType() const { return m_type; }

void Animal::setThrowStrength(float strength) { m_throwStrength = strength; }

void Animal::setPlayerRange(float range) { m_playerRange = range; }

void Animal::setFruitRange(float range) { m_fruitRange = range; }

void Animal::setFruitType(FruitType type) { m_fruitType = type; }

void Animal::setRequiredFruitCount(int count) { m_nrRequiredFruits = count; }

void Animal::setSleepPosition(float3 position) { m_sleepPos = position; }

void Animal::setType(Animal::Type type) {
	switch (type) {
	case Animal::Bear:
		load("Bear");
		break;
	case Animal::Goat:
		load("Goat");
		break;
	case Animal::Gorilla:
		load("Gorilla");
		break;
	}
}

bool Animal::notBribed() const { return m_nrFruitsTaken < m_nrRequiredFruits; }

void Animal::setAttacked(bool attacked) { m_hasAttacked = attacked; }

void Animal::makeAngrySound() {
	AudioController* ac = AudioController::getInstance();
	string animal = getModelName();
	if (!m_hasAttacked) {
		if (animal == "Bear") {
			ac->play("bear-push", AudioController::SoundType::Effect);
		}
		if (animal == "Goat") {
			ac->play("goat-push", AudioController::SoundType::Effect);
		}
		if (animal == "Gorilla") {
			ac->play("gorilla-push", AudioController::SoundType::Effect);
		}
		m_hasAttacked = true;
	}
}

void Animal::makeHappySound() {
	AudioController* ac = AudioController::getInstance();
	string animal = getModelName();
	if (!m_isSatisfied) {
		if (animal == "Bear") {
			ac->play("bear-happy", AudioController::SoundType::Effect);
		}
		if (animal == "Goat") {
			ac->play("goatr-happy", AudioController::SoundType::Effect);
		}
		if (animal == "Gorilla") {
			ac->play("gorilla-happy", AudioController::SoundType::Effect);
		}
		m_isSatisfied = true;
	}
}

void Animal::makeEatingSound() {
	AudioController* ac = AudioController::getInstance();
	string animal = getModelName();
	if (animal == "Bear") {
		ac->play("bear-eating", AudioController::SoundType::Effect);
	}
	if (animal == "Goat") {
		ac->play("goat-eating", AudioController::SoundType::Effect);
	}
	if (animal == "Gorilla") {
		ac->play("gorilla-eating", AudioController::SoundType::Effect);
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

void Animal::draw_onlyAnimal() {
	Entity::draw(); // Draw the animal
}

void Animal::reset() { 
	setPosition(m_origin);
	setRotation(float3(0, m_startRotation, 0));
	m_walkTimeTracker = 3;
	m_walkToPos = float3(0.);
	m_nrFruitsTaken = 0;

	//bubble
	m_thoughtBubble.setCurrentMaterial(3 * m_fruitType + m_nrRequiredFruits - 1);
	m_thoughtBubbleOffset =
		(getBoundingBoxPos() - getPosition()) + float3(0.f, getHalfSizes().y + 0.5f, 0.f);
	float3 topPos = getPosition() + m_thoughtBubbleOffset;
	m_thoughtBubble.setPosition(topPos);
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
	float3 playerToAnimal = getBoundingBoxPos() - playerPos;
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
