#include "PlayingState.h"
#include "PauseState.h"
#include "PushdownState.h"
#include "VictoryState.h"
#include "LossState.h"
#include "Debug.h"
#include "Window.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult PlayingState::OnUpdate(float dt, PushdownState** newState)
{
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		*newState = new PauseState();
		return PushdownResult::Push;
	}
	if (won) {
		*newState = new VictoryState(objectives, maxObjectives, timeTaken, timeAllotted);
		return PushdownResult::Push;
	}
	if (lost) {
		*newState = new LossState();
		return PushdownResult::Push;
	}

	return PushdownResult::NoChange;
}
