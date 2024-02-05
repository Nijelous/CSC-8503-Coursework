#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class PlayingState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	public:
		PlayingState(std::string level, bool editLevel) { this->level = level; this->editLevel = editLevel; }
		std::string LoadLevel() { 
			if (!levelLoaded) {
				levelLoaded = true;
				return level;
			}
			else return "Loaded";
		}
		bool IsEditing() { return editLevel; }
		void Victory(int objectives, int maxObjectives, float timeTaken, float timeAllotted) {
			won = true; 
			this->objectives = objectives;
			this->maxObjectives = maxObjectives;
			this->timeTaken = timeTaken;
			this->timeAllotted = timeAllotted;
		}
		void Loss() { lost = true; }
	protected:
		std::string level;
		bool levelLoaded = false;
		bool editLevel;
		bool won = false;
		bool lost = false;
		int objectives;
		int maxObjectives;
		float timeTaken;
		float timeAllotted;

	};
}

