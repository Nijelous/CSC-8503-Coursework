#pragma once
#include "GameObject.h"
namespace NCL::CSC8503 {
	class BombObject : public GameObject {
	public:
		BombObject(float timeToBlow) { name = "Bomb"; this->timeToBlow = timeToBlow; }
		~BombObject(){}
		void OnCollisionBegin(GameObject* otherObject) override;
		void UpdateObject(float dt, GameWorld& world) override;
	protected:
		bool hitObject = false;
		bool exploding = false;
		float timeToBlow;
		float explodingTime = 1.0f;
		Vector3 explosionPos;
	};
}
