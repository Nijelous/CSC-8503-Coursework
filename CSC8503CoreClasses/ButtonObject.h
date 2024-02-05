#pragma once
#include "GameObject.h"
namespace NCL::CSC8503 {
	class ButtonObject : public GameObject {
	public:
		ButtonObject(int flag) { buttonID = flag; name = "Button"; }
		~ButtonObject() {}
		void UpdateObject(float dt, GameWorld& world) override;
		void ToggleButton() { buttonPressed = true; }
		void SetButtonID(int id) { buttonID = abs(id%4); }
		int GetButtonID() { return buttonID; }
	protected:
		int buttonID;
		bool buttonPressed = false;
	};
}