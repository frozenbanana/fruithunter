#pragma once
#include <iostream>
#include <string>
using namespace std;

class StateItem {
public:
	enum State {
		PlayState,
		PauseState,
		SettingState,
		ControlState,
		MainState,
		EndRoundState,
		EditorState,
		Size
	};
	const string m_stateStrings[State::Size] = { "PlayState", "PauseState", "SettingState",
		"ControlState", "MainState", "EndRoundState", "EditorState" };
	struct Request {
		enum Type { PUSH, POP, NONE } type = NONE;
		State state = State::Size;
		bool restart = false;
	};

protected:
	State m_state;
	Request m_request;

public:
	StateItem(State state);
	virtual ~StateItem();

	StateItem::State getState() const;
	string getStateAsString() const;

	virtual void init() = 0;
	virtual void update(double dt) = 0;
	virtual void draw() = 0;
	virtual void play() = 0;
	virtual void pause() = 0;
	virtual void restart() = 0;

	StateItem::Request popRequest();
	/* Add state to stack*/
	void push(StateItem::State state);
	/* Go to previous state */
	void pop(bool restart);
	/* Jump back to state */
	void pop(StateItem::State state, bool restart);

};
