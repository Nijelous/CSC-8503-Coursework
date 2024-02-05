#include "PlayerObject.h"
#include "PhysicsObject.h"
#include "Debug.h"
#include "GameWorld.h"
#include "ExplosiveObject.h"
#include "ButtonObject.h"

using namespace NCL;
using namespace CSC8503;

void PlayerObject::OnCollisionBegin(GameObject* otherObject) {
	if (otherObject->GetName() == "Floor") {
		Vector3 linearVel = GetPhysicsObject()->GetLinearVelocity() * Vector3(1, 0, 1);
		GetPhysicsObject()->SetLinearVelocity(linearVel);
		floorTimer = 0.25f;
		onFloor = true;
	}
	else if (otherObject->GetName() == "Enemy") {
		isDead = true;
	}
}

void PlayerObject::OnCollisionEnd(GameObject* otherObject) {
	if (otherObject->GetName() == "Floor") {
		onFloor = false;
	}
}

void PlayerObject::UpdateObject(float dt, GameWorld& world) {
	if (isDead) {
		world.GameOver();
		return;
	}
	if (jumpPressed > 0) jumpPressed -= dt;
	if (!onFloor && floorTimer > 0) floorTimer -= dt;
	Vector3 rayDir = GetTransform().GetPosition() - world.GetMainCamera().GetPosition();
	Ray ray(GetTransform().GetPosition() + Vector3(0, 1, 0), rayDir.Normalised());

	RayCollision closestCollision;
	if (world.Raycast(ray, closestCollision, true, this) && closestCollision.rayDistance < 10) {
		hoveredObject = (GameObject*)closestCollision.node;
		Debug::DrawCube(hoveredObject->GetTransform().GetScale() / 2, hoveredObject->GetTransform().GetPosition());
	}
	else {
		hoveredObject = nullptr;
	}
	Debug::Print(std::format("Objectives obtained: {0}/{1}", collectedGoals, world.GetGoalTotal()), Vector2(70, 3), 
		collectedGoals == world.GetGoalTotal() ? Vector4(0, 1, 0, 1) : Vector4(1, 1, 1, 1), 12.0f);
	if (hoveredObject) {
		if (hoveredObject->GetName() == "Bomb") {
			Debug::Print("E: Pick up Bomb", Vector2(70, 70), Vector4(1, 1, 1, 1), 15.0f);
		}
		else if (hoveredObject->GetName() == "Wall" && selectedItem == 1) {
			Debug::Print("LClick: Place C4", Vector2(70, 70), Vector4(1, 1, 1, 1), 15.0f);
		}
		else if (hoveredObject->GetName() == "C4") {
			Debug::Print("E: Pick up C4", Vector2(70, 70), Vector4(1, 1, 1, 1), 15.0f);
		}
		else if (hoveredObject->GetName() == "Goal") {
			Debug::Print("E: Pick up objective", Vector2(70, 70), Vector4(1, 1, 1, 1), 15.0f);
		}
		else if (hoveredObject->GetName() == "Button") {
			Debug::Print("E: Toggle this colour door", Vector2(70, 70), Vector4(1, 1, 1, 1), 15.0f);
		}
	}
	else if (selectedItem == 1 && !placedC4.empty()) {
		Debug::Print("LClick: Explode C4", Vector2(70, 70), Vector4(1, 1, 1, 1), 15.0f);
	}
}

void PlayerObject::Jump() {
	GetPhysicsObject()->AddForce(Vector3(0, 100, 0));
	floorTimer = 0;
}

void PlayerObject::Interact(GameWorld& world) { //E
	if (hoveredObject) {
		if (hoveredObject->GetName() == "Bomb") {
			world.RemoveGameObject(hoveredObject, true);
			hoveredObject = nullptr;
			bombCount++;
		}
		else if (hoveredObject->GetName() == "C4") {
			placedC4.erase(std::remove(placedC4.begin(), placedC4.end(), hoveredObject), placedC4.end());
			world.RemoveGameObject(hoveredObject, true);
			hoveredObject = nullptr;
			c4Count++;
		}
		else if (hoveredObject->GetName() == "Goal") {
			world.RemoveGameObject(hoveredObject, true);
			hoveredObject = nullptr;
			collectedGoals++;
		}
		else if (hoveredObject->GetName() == "Button") {
			ButtonObject* obj = (ButtonObject*)hoveredObject;
			obj->ToggleButton();
		}
	}
}

int PlayerObject::UseItem() { //Left Click
	switch (selectedItem) {
	case 0:
		if (bombCount > 0) {
			bombCount--;
			return 0;
		}
		break;
	case 1:
		if (hoveredObject) {
			if (hoveredObject->GetName() == "Wall") {
				c4Count--;
				return 1;
			}
		}
		if (!placedC4.empty()) {
			explodedC4Positions.clear();
			explodedWallPositions.clear();
			for (int i = 0; i < placedC4.size(); i++) {
				explodedC4Positions.push_back(placedC4[i]->GetTransform().GetPosition());
				explodedWallPositions.push_back(placedC4[i]->GetWall()->GetTransform());
				placedC4[i]->Explode();
			}
			placedC4.clear();
			return 2;
		}
		break;
	}
	return -1;
}
