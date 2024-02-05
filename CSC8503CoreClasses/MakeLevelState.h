#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class MakeLevelState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	protected:
		int optionSelect = 0;
	};
}

