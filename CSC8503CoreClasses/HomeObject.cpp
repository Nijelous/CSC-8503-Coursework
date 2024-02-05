#include "HomeObject.h"
#include "GameWorld.h"
#include "../CSC8503/PlayerObject.h"

using namespace NCL::CSC8503;

void HomeObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName() == "Player") {
		isCollidingWithPlayer = true;
	}
}

void HomeObject::OnCollisionEnd(GameObject* otherObject) {
	if (otherObject->GetName() == "Player") {
		isCollidingWithPlayer = false;
	}
}

void HomeObject::UpdateObject(float dt, GameWorld& world) {
	if (isCollidingWithPlayer) {
		PlayerObject* obj = (PlayerObject*)world.GetPlayer();
		if (obj->GetCollectedGoals() >= 1) {
			world.GameOver();
		}
	}
}
