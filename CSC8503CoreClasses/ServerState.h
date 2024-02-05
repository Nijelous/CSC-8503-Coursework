#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class ServerState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	};
}
