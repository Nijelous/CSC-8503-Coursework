#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class VictoryState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	public:
		VictoryState(int objectives, int maxObjectives, int timeTaken, int timeAllotted) {
			this->objectives = objectives;
			this->maxObjectives = maxObjectives;
			this->timeTaken = timeTaken;
			this->timeAllotted = timeAllotted;
		}
		void OnAwake() override;
	protected:
		float tempCounter = 0;
		int objectives;
		int maxObjectives;
		float timeTaken;
		float timeAllotted;
		int score = 0;
		int flags = 0;
	};
}
