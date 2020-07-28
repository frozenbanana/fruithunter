#include "EditorState.h"
#include "Input.h"
#include "StateHandler.h"
#include "ErrorLogger.h"

EditorState::EditorState() { initialize(); }

EditorState::~EditorState() {}

void EditorState::initialize() { m_name = "Editor State"; }

void EditorState::update() {
	//Input::getInstance()->setMouseModeRelative();
	if (Input::getInstance()->keyPressed(m_mouseMode_switch))
		m_mouseMode = !m_mouseMode; 
	if (m_mouseMode)
		Input::getInstance()->setMouseModeRelative();
	else
		Input::getInstance()->setMouseModeAbsolute();

	editor.update();
}

void EditorState::handleEvent() {
	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		StateHandler::getInstance()->changeState(StateHandler::PAUSE);
	}
}

void EditorState::pause() { ErrorLogger::log(m_name + " pause() called."); }

void EditorState::play() {
	Input::getInstance()->setMouseModeRelative();
	ErrorLogger::log(m_name + " play() called.");

	editor.load(m_sceneToLoad);
}

void EditorState::draw() { editor.draw(); }
