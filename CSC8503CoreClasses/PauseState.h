#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class PauseState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	};
}

