#include "PauseState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult PauseState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("PAUSED", Vector2(30, 30), Vector4(0, 0, 1, 1), 30.0f);
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::DoublePop;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}
