#include "EditorState.h"
#include "Input.h"
#include "ErrorLogger.h"

EditorState::EditorState() : StateItem(State::EditorState) {}

EditorState::~EditorState() {}

void EditorState::init() { m_editor.clear(); }

void EditorState::update() {
	Input* ip = Input::getInstance();

	if (ip->keyPressed(m_mouseMode_switch))
		m_mouseMode = !m_mouseMode;
	if (m_mouseMode)
		ip->setMouseModeRelative();
	else
		ip->setMouseModeAbsolute();

	if (m_testing) {
		m_testingEnvironment.update();
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::MenuItem("Reset")) {
				m_testingEnvironment.reset();
				m_mouseMode = true;
			}
			if (ImGui::MenuItem("Stop(F5)") || ip->keyPressed(Keyboard::F5)) {
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

	if (Input::getInstance()->keyPressed(Keyboard::Keys::Escape)) {
		push(State::PauseState);
	}
}

void EditorState::pause() { }

void EditorState::play() {  }

void EditorState::restart() {}

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