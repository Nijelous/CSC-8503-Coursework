#include "SelectLevelState.h"
#include "PushdownState.h"
#include "PlayingState.h"
#include "ShowHighScoresState.h"
#include "Debug.h"
#include "Window.h"
#include <filesystem>

using namespace NCL::CSC8503;

PushdownState::PushdownResult SelectLevelState::OnUpdate(float dt, PushdownState** newState)
{
	if (levels.size() > 9) {
		int scroll = -Window::GetMouse()->GetWheelMovement();
		optionSelect = std::min((int)levels.size() - 9, std::max(0, optionSelect + scroll));
	}
	Debug::Print("CHOOSE A LEVEL", Vector2(30, 20), Vector4(1, 0, 0, 1), 30.0f);
	for (int i = optionSelect; i < optionSelect + 9 && i < levels.size(); i++) {
		Debug::Print(std::to_string(i + 1 - optionSelect) + ": " + levels[i].substr(7), Vector2(30, 30 + ((i - optionSelect) * 3)));
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM1)) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect]);
		else *newState = new PlayingState(levels[optionSelect], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM2) && levels.size() > optionSelect + 1) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 1]);
		else *newState = new PlayingState(levels[optionSelect + 1], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM3) && levels.size() > optionSelect + 2) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 2]);
		else *newState = new PlayingState(levels[optionSelect + 2], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM4) && levels.size() > optionSelect + 3) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 3]);
		else *newState = new PlayingState(levels[optionSelect + 3], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM5) && levels.size() > optionSelect + 4) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 4]);
		else *newState = new PlayingState(levels[optionSelect + 4], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM6) && levels.size() > optionSelect + 5) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 5]);
		else *newState = new PlayingState(levels[optionSelect + 5], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM7) && levels.size() > optionSelect + 6) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 6]);
		else *newState = new PlayingState(levels[optionSelect + 6], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM8) && levels.size() > optionSelect + 7) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 7]);
		else *newState = new PlayingState(levels[optionSelect + 7], nextFlags & 1);
		return PushdownResult::Push;
	}
	else if (Window::GetKeyboard()->KeyPressed(KeyCodes::NUM9) && levels.size() > optionSelect + 8) {
		if (nextFlags & (1 << 1)) *newState = new ShowHighScoresState(levels[optionSelect + 8]);
		else *newState = new PlayingState(levels[optionSelect + 8], nextFlags & 1);
		return PushdownResult::Push;
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::ESCAPE)) {
		return PushdownResult::Pop;
	}

	return PushdownResult::NoChange;
}

void SelectLevelState::OnAwake() {
	optionSelect = 0;
	levels.clear();
	for (const auto& entry : std::filesystem::directory_iterator("Levels")) levels.push_back(entry.path().string());
}
