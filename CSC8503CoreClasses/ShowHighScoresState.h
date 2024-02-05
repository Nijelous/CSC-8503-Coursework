#pragma once
#include "PushdownState.h"
namespace NCL::CSC8503 {
	class ShowHighScoresState : public PushdownState {
		PushdownResult OnUpdate(float dt, PushdownState** newState) override;
	public:
		ShowHighScoresState(std::string levelName) { this->levelName = levelName; }
		void AddHighScore(int highScore) { highScores.push_back(highScore); }
		bool NeedsHighScores() { bool hs = needsHighScores; needsHighScores = false; return hs; }
		std::string GetLevelName() { return levelName; }
	protected:
		bool needsHighScores = true;
		std::string levelName;
		std::vector<int> highScores;
		int optionSelect = 0;
	};
}
