#include "EditorState.h"
#include "Input.h"
#include "StateHandler.h"
#include "ErrorLogger.h"

EditorState::EditorState() { initialize(); }

EditorState::~EditorState() {}

void EditorState::initialize() { m_name = "Editor State"; }

void EditorState::update() {
	Input* ip = Input::getInstance();

	// Input::getInstance()->setMouseModeRelative();
	if (Input::getInstance()->keyPressed(m_mouseMode_switch))
		m_mouseMode = !m_mouseMode;
	if (m_mouseMode)
		Input::getInstance()->setMouseModeRelative();
	else
		Input::getInstance()->setMouseModeAbsolute();

	if (m_testing) {
		m_testingEnvironment.update();
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::MenuItem("Reset")) {
				m_testingEnvironment.reset();
				m_mouseMode = true;
			}
			if (ImGui::MenuItem("Stop")) {
				m_editor.reset();
				m_testing = false;
				m_mouseMode = false;
			}
			ImGui::EndMainMenuBar();
		}
	}
	else {
		m_editor.update();
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::MenuItem("Run(F5)") || ip->keyPressed(Keyboard::F5)) {
				m_testingEnvironment.reset();
				m_testing = true;
				m_mouseMode = true;
			}
			ImGui::EndMainMenuBar();
		}
	}
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
	m_editor.clear();
}

void EditorState::draw() {
	if (m_testing) {
		m_testingEnvironment.setup_shadow();
		m_testingEnvironment.draw_shadow();
		m_testingEnvironment.setup_color();
		m_testingEnvironment.draw_color();
		m_testingEnvironment.draw_hud();
	}
	else {
		m_editor.draw();
	}
}