#include "EnemyObject.h"
#include "GameWorld.h"
#include "State.h"
#include "StateTransition.h"
#include "PhysicsObject.h"

using namespace NCL::CSC8503;

EnemyObject::EnemyObject() {
	name = "Enemy";
	sm = new StateMachine();
	State* Wander = new State([&](float dt)->void {
		GetPhysicsObject()->AddForce(wanderDirection * 2);
		wanderCount -= dt;
		});
	State* WalkToPlayer = new State([&](float dt)->void {
		GetPhysicsObject()->AddForce((playerPosition - GetTransform().GetPosition()).Normalised() * 3);
		});
	StateTransition* SeePlayer = new StateTransition(Wander, WalkToPlayer, [&](void)->bool {
		if (wanderCount > 0) return false;
		wanderCount = 5;
		wanderDirection = Vector3(rand(), rand(), rand()).Normalised();
		playerPosition = world->GetPlayer()->GetTransform().GetPosition();
		return CanSeePlayer();
		});
	StateTransition* CantSeePlayer = new StateTransition(WalkToPlayer, Wander, [&](void)->bool {
		if ((playerPosition - GetTransform().GetPosition()).Length() > 2) return false;
		playerPosition = world->GetPlayer()->GetTransform().GetPosition();
		return !CanSeePlayer();
		});
	sm->AddState(Wander);
	sm->AddState(WalkToPlayer);
	sm->AddTransition(SeePlayer);
	sm->AddTransition(CantSeePlayer);
}

void EnemyObject::UpdateObject(float dt, GameWorld& world) {
	if (!this->world) this->world = &world;
	if (!stunned) sm->Update(dt);
	else {
		stunnedCount -= dt;
		if (stunnedCount <= 0) {
			stunned = false;
			playerPosition = GetTransform().GetPosition();
			wanderCount = 0;
		}
		std::cout << "Stunned for " << stunnedCount << " seconds!\n";
	}
}

bool EnemyObject::CanSeePlayer()
{
	Vector3 rayDir = (playerPosition - GetTransform().GetPosition()).Normalised();
	Ray ray(GetTransform().GetPosition(), rayDir.Normalised());

	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true, this)) {
		GameObject* node = (GameObject*)closestCollision.node;
		return node->GetName() == "Player";
	}
	else return false;
}
