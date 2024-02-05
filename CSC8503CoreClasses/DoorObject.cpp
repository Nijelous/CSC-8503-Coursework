#include "DoorObject.h"
#include "GameWorld.h"
#include "PhysicsObject.h"

using namespace NCL::CSC8503;

void DoorObject::UpdateObject(float dt, GameWorld& world) {
	if (world.IsDoorOpen(button) != opened) {
		std::cout << "Opening doors " << button << "\n";
		opened = !opened;
		if (opened) {
			GetPhysicsObject()->SetInverseMass(1.0f);
		}
		else {
			GetPhysicsObject()->SetInverseMass(0.0f);
		}
	}
}
