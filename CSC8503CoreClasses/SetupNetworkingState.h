#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class SetupNetworkingState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	protected:
		int optionSelect = 0;
	};
}
