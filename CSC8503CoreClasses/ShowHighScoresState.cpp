#include "ShowHighScoresState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"

using namespace NCL::CSC8503;

PushdownState::PushdownResult ShowHighScoresState::OnUpdate(float dt, PushdownState** newState) {
	if (highScores.size() > 15) {
		int scroll = -Window::GetMouse()->GetWheelMovement();
		optionSelect = std::min((int)highScores.size() - 15, std::max(0, optionSelect + scroll));
	}
	Debug::Print("HIGH SCORES", Vector2(30, 20), Vector4(1, 0, 0, 1), 30.0f);
	for (int i = optionSelect; i < optionSelect + 15 && i < highScores.size(); i++) {
		Debug::Print(std::to_string(i + 1 - optionSelect) + ": " + std::to_string(highScores[i]), Vector2(30, 30 + ((i - optionSelect) * 3)));
	}

	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}