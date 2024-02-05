#include "BombObject.h"
#include "GameWorld.h"
#include "RenderObject.h"
#include "PhysicsObject.h"
#include "EnemyObject.h"
using namespace NCL::CSC8503;

void BombObject::OnCollisionBegin(GameObject* otherObject) {
	if (!exploding) {
		hitObject = true; 
	}
	else {
		if (otherObject->GetName() == "Enemy") {
			EnemyObject* obj = (EnemyObject*)otherObject;
			obj->StunEnemy();
		}
		if (otherObject->GetBoundingVolume()->type == VolumeType::OBB && otherObject->GetPhysicsObject()->GetInverseMass() > 0) {
			otherObject->GetPhysicsObject()->ApplyLinearImpulse((explosionPos - otherObject->GetTransform().GetPosition()));
		}
	}
}

void BombObject::UpdateObject(float dt, GameWorld& world) {
	if (timeToBlow <= 0 || hitObject) {
		exploding = true;
		explosionPos = GetTransform().GetPosition();
		GetRenderObject()->SetColour(Vector4(1.0f, 0.5f, 0.0f, 1.0f));
		GetPhysicsObject()->ClearForces();
		GetPhysicsObject()->SetInverseMass(0);
	}
	if (exploding) {
		GetTransform().SetPosition(explosionPos);
		explodingTime -= dt;
		if (explodingTime <= 0) world.RemoveGameObject(this, true);
		else {
			GetTransform().SetScale(GetTransform().GetScale() + Vector3(dt * 10, dt * 10, dt * 10));
			SphereVolume* sv = (SphereVolume*)GetBoundingVolume();
			sv->SetRadius(sv->GetRadius() + (dt * 10));
		}
	}
	else {
		timeToBlow -= dt;
	}
}
