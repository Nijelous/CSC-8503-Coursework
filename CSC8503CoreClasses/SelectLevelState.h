#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class SelectLevelState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
		void OnAwake() override;
	public:
		SelectLevelState(int nextFlags) { this->nextFlags = nextFlags; }
	protected:
		int optionSelect = 0;
		std::vector<std::string> levels;
		int nextFlags;
	};
}

