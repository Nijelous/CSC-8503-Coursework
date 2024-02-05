#include "LossState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult LossState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("YOU LOSE", Vector2(30, 20), Vector4(1, 0, 0, 1), 30.0f);
	Debug::Print("Press Enter to go back", Vector2(30, 40), Vector4(0, 0, 1, 1));
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
		return PushdownResult::DoublePop;
	}

	return PushdownResult::NoChange;
}
