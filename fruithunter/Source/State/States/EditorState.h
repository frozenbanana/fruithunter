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

	void init() override;
	void update(double dt) override;
	void draw() override;

	void pause() override;
	void play() override;
	void restart() override;
};
