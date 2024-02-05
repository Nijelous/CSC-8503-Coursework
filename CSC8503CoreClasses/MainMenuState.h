#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class MainMenuState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	public:
		MainMenuState(bool offline) { this->offline = offline; }
		bool IsOffline() { return offline; }
	protected:
		bool offline;
		int optionSelect = 0;
	};
}

