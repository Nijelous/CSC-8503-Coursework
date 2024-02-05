#include "SetupNetworkingState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"
#include "MainMenuState.h"
#include "ServerState.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult SetupNetworkingState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("CLIENT OR SERVER", Vector2(30, 20), Vector4(1, 0, 0, 1), 30.0f);
	if (optionSelect == 0) Debug::Print(">Client", Vector2(20, 40), Vector4(0, 1, 0, 1));
	else Debug::Print("Client", Vector2(20, 40), Vector4(1, 1, 1, 1));

	if (optionSelect == 1) Debug::Print(">Server", Vector2(50, 40), Vector4(0, 1, 0, 1));
	else Debug::Print("Server", Vector2(50, 40), Vector4(1, 1, 1, 1));

	if(optionSelect == 2) Debug::Print(">Offline", Vector2(35, 60), Vector4(0, 1, 0, 1));
	else Debug::Print("Offline", Vector2(35, 60), Vector4(1, 1, 1, 1));

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::LEFT)) {
		optionSelect = 0;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RIGHT)) {
		optionSelect = 1;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::DOWN)) {
		optionSelect = 2;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
		switch (optionSelect) {
		case 0:
			*newState = new MainMenuState(false);
			return PushdownResult::Push;
		case 1:
			*newState = new ServerState();
			return PushdownResult::Push;
		case 2:
			*newState = new MainMenuState(true);
			return PushdownResult::Push;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}
