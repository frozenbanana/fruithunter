#pragma once
#include "State.h"
#include "SceneEditorManager.h"

class EditorState : public State {
private:
	SceneEditorManager editor;
	const string m_sceneToLoad = "scene1";
	Keyboard::Keys m_mouseMode_switch = Keyboard::H;
	bool m_mouseMode = true;

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
