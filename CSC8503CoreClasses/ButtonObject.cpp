#include "ButtonObject.h"
#include "GameWorld.h"

using namespace NCL::CSC8503;

void ButtonObject::UpdateObject(float dt, GameWorld& world) {
	if (buttonPressed) {
		world.SetDoorFlag(buttonID);
		buttonPressed = false;
	}
}
