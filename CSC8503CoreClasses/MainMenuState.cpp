#include "MainMenuState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"
#include "MakeLevelState.h"
#include "SelectLevelState.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult MainMenuState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("RIPPED OFF MAKER", Vector2(30, 20), Vector4(1, 0, 0, 1), 30.0f);
	if (optionSelect == 0) Debug::Print(">Play Level", Vector2(30, 26), Vector4(0, 1, 0, 1));
	else Debug::Print("Play Level", Vector2(30, 26), Vector4(1, 1, 1, 1));

	if (optionSelect == 1) Debug::Print(">Make Level", Vector2(30, 29), Vector4(0, 1, 0, 1));
	else Debug::Print("Make Level", Vector2(30, 29), Vector4(1, 1, 1, 1));

	if (optionSelect == 2) Debug::Print(">High Scores", Vector2(30, 32), Vector4(0, 1, 0, 1));
	else Debug::Print("High Scores", Vector2(30, 32), Vector4(1, 1, 1, 1));

	if (optionSelect == 3) Debug::Print(">Quit", Vector2(30, 35), Vector4(0, 1, 0, 1));
	else Debug::Print("Quit", Vector2(30, 35), Vector4(1, 1, 1, 1));

	

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::UP)) {
		optionSelect--;
		if (optionSelect < 0) optionSelect = 0;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
		optionSelect++;
		if (optionSelect > 3) optionSelect = 3;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
		switch (optionSelect) {
		case 0:
			*newState = new SelectLevelState(0);
			return PushdownResult::Push;
		case 1:
			*newState = new MakeLevelState();
			return PushdownResult::Push;
		case 2:
			*newState = new SelectLevelState(2);
			return PushdownResult::Push;
		case 3:
			return PushdownResult::DoublePop;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::DoublePop;
	}

	return PushdownResult::NoChange;
}
