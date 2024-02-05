#pragma once
#include "GameObject.h"
#include <StateMachine.h>
namespace NCL::CSC8503 {
	class EnemyObject : public GameObject {
	public:
		EnemyObject();
		~EnemyObject(){}
		void UpdateObject(float dt, GameWorld& world) override;
		void StunEnemy() { stunned = true; stunnedCount = 10; }
	protected:
		bool CanSeePlayer();
		bool stunned = false;
		float stunnedCount = 0;
		StateMachine* sm;
		float wanderCount = 0;
		Vector3 wanderDirection;
		Vector3 playerPosition;
		GameWorld* world = nullptr;
	};
}

