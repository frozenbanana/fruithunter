#pragma once
#include "StateItem.h"
#include "SceneEditorManager.h"

class EditorState : public StateItem {
private:
	SceneEditorManager m_editor;
	SceneManager m_testingEnvironment;
	Keyboard::Keys m_mouseMode_switch = Keyboard::H;
	bool m_mouseMode = false;
	bool m_testing = false;

public:
	EditorState();
	~EditorState();

	void init();
	void update();
	void draw();

	void pause();
	void play();
	void restart();
};
