#include "ServerState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult ServerState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("THIS IS THE SERVER", Vector2(30, 20), Vector4(0, 0, 1, 1), 30.0f);

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::DoublePop;
	}

	return PushdownResult::NoChange;
}
