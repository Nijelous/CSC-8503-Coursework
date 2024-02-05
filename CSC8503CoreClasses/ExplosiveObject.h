#pragma once
#include "GameObject.h"
namespace NCL::CSC8503 {
	class ExplosiveObject : public GameObject {
	public:
		ExplosiveObject(std::string name, GameObject* wall) { this->name = name; this->wall = wall; }
		~ExplosiveObject() {}
		void UpdateObject(float dt, GameWorld& world) override;
		void Explode();
		GameObject* GetWall() { return wall; }
	protected:
		bool setToExplode = false;
		GameObject* wall = nullptr;
	};
}
