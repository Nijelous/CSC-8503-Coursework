#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class LossState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	};
}
