#include "ExplosiveObject.h"
#include "GameWorld.h"
using namespace NCL::CSC8503;

void ExplosiveObject::UpdateObject(float dt, GameWorld& world) {
	if (setToExplode) {
		if (wall->IsActive()) {
			world.RemoveGameObject(wall, true);
			wall->Deactivate();
		}
		world.RemoveGameObject(this, true);
	}
}

void ExplosiveObject::Explode() {
	setToExplode = true;
}
