#include "VictoryState.h"
#include "PushdownState.h"
#include "Debug.h"
#include "Window.h"
#include <format>

using namespace NCL::CSC8503;

PushdownState::PushdownResult VictoryState::OnUpdate(float dt, PushdownState** newState)
{
	Debug::Print("YOU WIN", Vector2(30, 20), Vector4(0, 1, 0, 1), 30.0f);
	if (flags & (1 << 4)) {
		Debug::Print(std::format("Objectives collected: {}", objectives), Vector2(30, 26), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Objectives missed: {}", maxObjectives - objectives), Vector2(30, 29), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Time taken: {:.2f}",timeAllotted - timeTaken), Vector2(30, 32), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Score: {}", score), Vector2(30, 35), Vector4(1, 1, 1, 1));
		Debug::Print("Press Enter to go back", Vector2(30, 70), Vector4(0, 0, 1, 1));
		if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
			return PushdownResult::DoublePop;
		}
	}
	else if (flags & (1 << 3)) {
		tempCounter += dt*400;
		Debug::Print(std::format("Objectives collected: {}", objectives), Vector2(30, 26), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Objectives missed: {}", maxObjectives - objectives), Vector2(30, 29), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Time taken: {:.2f}", timeAllotted - timeTaken), Vector2(30, 32), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Score: {:.0f}", tempCounter), Vector2(30, 35), Vector4(0, 1, 0, 1));
		if (tempCounter >= score) {
			tempCounter = 0;
			flags = flags | (1 << 4);
		}
	}
	else if (flags & (1 << 2)) {
		tempCounter += dt*20;
		Debug::Print(std::format("Objectives collected: {}", objectives), Vector2(30, 26), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Objectives missed: {}", maxObjectives - objectives), Vector2(30, 29), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Time taken: {:.2f}", tempCounter), Vector2(30, 32), Vector4(0, 1, 0, 1));
		if (tempCounter >= timeAllotted - timeTaken) {
			tempCounter = 0;
			flags = flags | (1 << 3);
		}
	}
	else if (flags & (1 << 1)) {
		tempCounter += dt;
		Debug::Print(std::format("Objectives collected: {}", objectives), Vector2(30, 26), Vector4(1, 1, 1, 1));
		Debug::Print(std::format("Objectives missed: {:.0f}", tempCounter), Vector2(30, 29), Vector4(0, 1, 0, 1));
		if (tempCounter >= maxObjectives - objectives) {
			tempCounter = 0;
			flags = flags | (1 << 2);
		}
	}
	else if (flags & 1) {
		tempCounter += dt;
		Debug::Print(std::format("Objectives collected: {:.0f}", tempCounter), Vector2(30, 26), Vector4(0, 1, 0, 1));
		if (tempCounter >= objectives) {
			tempCounter = 0;
			flags = flags | (1 << 1);
		}
	}
	else {
		tempCounter += dt;
		if (tempCounter >= 1.5f) {
			tempCounter = 0;
			flags = flags | 1;
		}
	}
	if (Window::GetKeyboard()->KeyPressed(KeyCodes::RETURN)) {
		flags = 31;
	}

	return PushdownResult::NoChange;
}

void VictoryState::OnAwake() {
	score = (timeTaken * 10) + (objectives * 300) - ((maxObjectives - objectives) * 400);
	if (score < 0) score = 0;
}
