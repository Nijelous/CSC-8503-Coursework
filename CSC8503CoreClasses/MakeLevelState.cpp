#include "MakeLevelState.h"
#include "PushdownState.h"
#include "PlayingState.h"
#include "SelectLevelState.h"
#include "Debug.h"
#include "Window.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult MakeLevelState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("MAKE LEVEL", Vector2(30, 20), Vector4(1, 0, 0, 1), 30.0f);
	if (optionSelect == 0) Debug::Print(">New Level", Vector2(20, 40), Vector4(0, 1, 0, 1));
	else Debug::Print("New Level", Vector2(20, 40), Vector4(1, 1, 1, 1));

	if (optionSelect == 1) Debug::Print(">Edit Existing Level", Vector2(50, 40), Vector4(0, 1, 0, 1));
	else Debug::Print("Edit Existing Level", Vector2(50, 40), Vector4(1, 1, 1, 1));

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::LEFT)) {
		optionSelect = 0;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RIGHT)) {
		optionSelect = 1;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
		switch (optionSelect) {
		case 0:
			*newState = new PlayingState("", true);
			return PushdownResult::Push;
		case 1:
			*newState = new SelectLevelState(1);
			return PushdownResult::Push;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}
