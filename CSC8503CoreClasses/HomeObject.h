#pragma once
#include "GameObject.h"
namespace NCL::CSC8503 {
	class HomeObject : public GameObject {
	public:
		HomeObject() { name = "Home"; }
		~HomeObject() {}
		void OnCollisionBegin(GameObject* otherObject) override;
		void OnCollisionEnd(GameObject* otherObject) override;
		void UpdateObject(float dt, GameWorld& world) override;
	protected:
		bool isCollidingWithPlayer = false;
	};
}
