#pragma once
#include "State.h"
#include "SceneEditorManager.h"

class EditorState : public State {
private:
	SceneEditorManager m_editor;
	SceneManager m_testingEnvironment;
	Keyboard::Keys m_mouseMode_switch = Keyboard::H;
	bool m_mouseMode = true;
	bool m_testing = false;

public:
	EditorState();
	~EditorState();

	void initialize();
	void update();
	void handleEvent();
	void pause();
	void play();
	void draw();
};
